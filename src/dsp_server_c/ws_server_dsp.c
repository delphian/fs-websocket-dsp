/**
 * @file ws_server_dsp.c
 * @brief WS protocol handler plugin for 'lws-fs-dsp'
 * @details Parse out websocket message and hand it of to fs-dsp subprotocol.
 */

#if !defined (LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#endif

#include <string.h>
#include "include/fs_ws_dsp.h"
#include "fs_ws_dsp_command.c"
#include "fs_ws_dsp_message.c"
#include "fs_ws_dsp_cmd_echo.c"
#include "fs_ws_dsp.c"

#define RING_DEPTH 1024 * 32

/* one of these created for each message fragment */
struct msg {
	void *payload; /* is malloc'd */
	size_t len;
	char binary;
	char first;
	char final;
};

struct session_data {
	struct lws_ring *frag_ring;
	uint32_t frag_tail;
	struct lws_ring *ring;
	uint32_t tail;
	uint32_t msglen;
	uint8_t completed:1;
	uint8_t flow_controlled:1;
	uint8_t write_consume_pending:1;
};

struct vhd_minimal_server_echo {
	struct lws_context *context;
	struct lws_vhost *vhost;
	int *interrupted;
	int *options;
};

static void __minimal_destroy_message(void *_msg)
{
	struct msg *msg = _msg;
	free(msg->payload);
	msg->payload = NULL;
	msg->len = 0;
}

#include <assert.h>
/**
 * @brief Websocket event callback.
 * 
 * @param[in]	wsi		What is an WSI?
 * @param[in]	reason	Event identification.
 * @param[in]	user	Session data.
 * @param[in]	in		Incoming data from client.
 * @param[in]	len		Byte length of data.
 */
static int callback_dsp(
	struct lws *wsi, 
	enum lws_callback_reasons reason,
	void *user, 
	void *in, 
	size_t len)
{
	struct session_data *session = (struct session_data *) user;
	/* Virtual host pointer */
	struct vhd_minimal_server_echo *vhost = 
			(struct vhd_minimal_server_echo *)
			lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));
	const struct msg *request; /* Client message */
	struct msg response; 	/* Resposne message */
	struct msg fragment;
	int m, ring_capacity, flags;

	switch (reason) {

	case LWS_CALLBACK_PROTOCOL_INIT:
		vhost = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct vhd_minimal_server_echo));
		if (!vhost)
			return -1;
		vhost->context = lws_get_context(wsi);
		vhost->vhost = lws_get_vhost(wsi);
		/* get the pointers we were passed in pvo */
		vhost->interrupted = (int *)lws_pvo_search((const struct lws_protocol_vhost_options *)in, "interrupted")->value;
		vhost->options = (int *)lws_pvo_search((const struct lws_protocol_vhost_options *)in, "options")->value;
		break;

	case LWS_CALLBACK_ESTABLISHED:
//		lwsl_warn("LWS_CALLBACK_ESTABLISHED\n");
		session->ring      = lws_ring_create(sizeof(struct msg), RING_DEPTH, __minimal_destroy_message);
		session->frag_ring = lws_ring_create(sizeof(struct msg), RING_DEPTH, __minimal_destroy_message);
		if (!session->ring || !session->frag_ring) {
			if (session->ring)
				lws_ring_destroy(session->ring);
			if (session->frag_ring)
				lws_ring_destroy(session->ring);
			return 1;
		}
		session->tail      = 0;
		session->frag_tail = 0;
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
//		lwsl_user("LWS_CALLBACK_SERVER_WRITEABLE\n");

		if (session->write_consume_pending) {
			/* perform the deferred fifo consume */
			lws_ring_consume_single_tail(session->ring, &session->tail, 1);
			session->write_consume_pending = 0;
		}

		request = lws_ring_get_element(session->ring, &session->tail);
		if (!request) {
//			lwsl_user(" (nothing in ring)\n");
			break;
		}
		flags = lws_write_ws_flags(
			request->binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT,
			request->first, request->final);
		/* Send to client. Notice we allowed for LWS_PRE in the payload already */
		m = lws_write(wsi, ((unsigned char *)request->payload) + LWS_PRE, request->len, (enum lws_write_protocol)flags);
		if (m < (int)request->len) {
			lwsl_err("ERROR %d writing to ws socket\n", m);
			return -1;
		}
//		lwsl_user(" wrote %d: flags: 0x%x first: %d final %d\n", m, flags, request->first, request->final);

		/*
		 * Workaround deferred deflate in pmd extension by only
		 * consuming the fifo entry when we are certain it has been
		 * fully deflated at the next WRITABLE callback.  You only need
		 * this if you're using pmd.
		 */
		session->write_consume_pending = 1;
		lws_callback_on_writable(wsi);
		/* Session ring has recovered room. turn off flow control. */
		if (session->flow_controlled &&
		    (int)lws_ring_get_count_free_elements(session->ring) > RING_DEPTH - 5) {
			lws_rx_flow_control(wsi, 1);
			session->flow_controlled = 0;
		}

		if ((*vhost->options & 1) && request && request->final)
			session->completed = 1;

		break;

	case LWS_CALLBACK_RECEIVE:
//		lwsl_user("LWS_CALLBACK_RECEIVE\n");
		/* If final fragment then concat all saved fragments and process message, otherwise add fragment to ring. */
    	if (lws_is_final_fragment(wsi)) {
			/* Compile contents of fragment ring into single array */
			char *message = malloc(session->msglen + len);
			char *dest = message;
			struct msg *compile_frag;
			compile_frag = (struct msg *) lws_ring_get_element(session->frag_ring, &session->frag_tail);
			while (compile_frag) {
				memcpy(dest, compile_frag->payload, compile_frag->len);
				dest += compile_frag->len;
				lws_ring_consume_single_tail(session->frag_ring, &session->frag_tail, 1);
				compile_frag = (struct msg *) lws_ring_get_element(session->frag_ring, &session->frag_tail);
			}
			memcpy(dest, in, len);
			struct fs_ws_dsp_message s_request = fs_ws_dsp_message_parse(message, session->msglen + len);
			struct fs_ws_dsp_message s_response = fs_ws_dsp_process(s_request);
			uint32_t response_len = fs_ws_dsp_message_serialize_size(s_response);
			char *response_payload = fs_ws_dsp_message_serialize(s_response);
			/* APPEND TO RING */
			fragment.first   = 1;
			fragment.final   = 1;
			fragment.binary  = 1;
			fragment.len     = response_len;
			fragment.payload = malloc(LWS_PRE + response_len);
			memcpy(fragment.payload + LWS_PRE, response_payload, response_len);
			if (!lws_ring_insert(session->ring, &fragment, 1)) {
				fprintf(stderr, "Response ring is full!\n");
				__minimal_destroy_message(&fragment);
			}
			free(message);
			fs_ws_dsp_message_free(s_response);
			fs_ws_dsp_message_free(s_request);
			lws_callback_on_writable(wsi);
			session->msglen = 0;
	    } else {
			/* Discard this fragment if we are out of room. */
			ring_capacity = (int)lws_ring_get_count_free_elements(session->frag_ring);
			if (!ring_capacity) {
				lwsl_user("dropping!\n");
				break;
			}
			fragment.first   = (char)lws_is_first_fragment(wsi);
			fragment.final   = (char)lws_is_final_fragment(wsi);
			fragment.binary  = (char)lws_frame_is_binary(wsi);
			fragment.len     = len;
			fragment.payload = malloc(len);
			if (!fragment.payload) {
				lwsl_user("OOM: dropping\n");
				break;
			}
			memcpy((char *)fragment.payload, in, len);
			if (!lws_ring_insert(session->frag_ring, &fragment, 1)) {
				__minimal_destroy_message(&fragment);
				lwsl_user("dropping!\n");
				break;
			}
			/* Ring is almost full. Switch on flow control. */
			if (ring_capacity < 3 && !session->flow_controlled) {
				session->flow_controlled = 1;
				lws_rx_flow_control(wsi, 0);
			}
			session->msglen += (uint32_t)len;
		}
		break;

	case LWS_CALLBACK_CLOSED:
		lwsl_user("LWS_CALLBACK_CLOSED\n");
		lws_ring_destroy(session->ring);
		if (*vhost->options & 1) {
			if (!*vhost->interrupted)
				*vhost->interrupted = 1 + session->completed;
			lws_cancel_service(lws_get_context(wsi));
		}
		break;

	default:
		break;
	}

	return 0;
}

#define LWS_PLUGIN_PROTOCOL_MINIMAL_SERVER_ECHO \
	{ \
		"lws-minimal-server-echo", \
		callback_dsp, \
		sizeof(struct session_data), \
		1024, \
		0, NULL, 0 \
	}
