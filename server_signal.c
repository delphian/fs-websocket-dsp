/*
 * ws protocol handler plugin for "lws-minimal-server-echo"
 *
 * Written in 2010-2019 by Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * The protocol shows how to send and receive bulk messages over a ws connection
 * that optionally may have the permessage-deflate extension negotiated on it.
 */

#if !defined (LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#endif

#include <string.h>
#include "include/fs_ws_signal.h"
#include "fs_ws_signal.c"

#define RING_DEPTH 4096

/* one of these created for each message */

struct msg {
	void *payload; /* is malloc'd */
	size_t len;
	char binary;
	char first;
	char final;
};

struct per_session_data__minimal_server_echo {
	struct lws_ring *ring;
	uint32_t msglen;
	uint32_t tail;
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

static void
__minimal_destroy_message(void *_msg)
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
static int callback_minimal_server_echo(
	struct lws *wsi, 
	enum lws_callback_reasons reason,
	void *user, 
	void *in, 
	size_t len)
{
	/* Session pointer */
	struct per_session_data__minimal_server_echo *session =
			(struct per_session_data__minimal_server_echo *)user;
	/* Virtual host pointer */
	struct vhd_minimal_server_echo *vhost = 
			(struct vhd_minimal_server_echo *)
			lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));
	const struct msg *request; /* Client message */
	struct msg response; 	/* Resposne message */
	int m, n, flags;

	switch (reason) {

	case LWS_CALLBACK_PROTOCOL_INIT:
		vhost = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
				lws_get_protocol(wsi),
				sizeof(struct vhd_minimal_server_echo));
		if (!vhost)
			return -1;

		vhost->context = lws_get_context(wsi);
		vhost->vhost = lws_get_vhost(wsi);

		/* get the pointers we were passed in pvo */

		vhost->interrupted = (int *)lws_pvo_search(
			(const struct lws_protocol_vhost_options *)in,
			"interrupted")->value;
		vhost->options = (int *)lws_pvo_search(
			(const struct lws_protocol_vhost_options *)in,
			"options")->value;
		break;

	case LWS_CALLBACK_ESTABLISHED:
		/* generate a block of output before travis times us out */
		lwsl_warn("LWS_CALLBACK_ESTABLISHED\n");
		session->ring = lws_ring_create(sizeof(struct msg), RING_DEPTH,
					    __minimal_destroy_message);
		if (!session->ring)
			return 1;
		session->tail = 0;
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:

		lwsl_user("LWS_CALLBACK_SERVER_WRITEABLE\n");

		if (session->write_consume_pending) {
			/* perform the deferred fifo consume */
			lws_ring_consume_single_tail(session->ring, &session->tail, 1);
			session->write_consume_pending = 0;
		}

		request = lws_ring_get_element(session->ring, &session->tail);
		if (!request) {
			lwsl_user(" (nothing in ring)\n");
			break;
		}

		flags = lws_write_ws_flags(
			request->binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT,
			request->first, request->final);

		/* notice we allowed for LWS_PRE in the payload already */
		m = lws_write(wsi, ((unsigned char *)request->payload) +
			LWS_PRE, request->len, (enum lws_write_protocol)flags);
		if (m < (int)request->len) {
			lwsl_err("ERROR %d writing to ws socket\n", m);
			return -1;
		}

		lwsl_user(" wrote %d: flags: 0x%x first: %d final %d\n",
				m, flags, request->first, request->final);
		/*
		 * Workaround deferred deflate in pmd extension by only
		 * consuming the fifo entry when we are certain it has been
		 * fully deflated at the next WRITABLE callback.  You only need
		 * this if you're using pmd.
		 */
		session->write_consume_pending = 1;
		lws_callback_on_writable(wsi);

		if (session->flow_controlled &&
		    (int)lws_ring_get_count_free_elements(session->ring) > RING_DEPTH - 5) {
			lws_rx_flow_control(wsi, 1);
			session->flow_controlled = 0;
		}

		if ((*vhost->options & 1) && request && request->final)
			session->completed = 1;

		break;

	case LWS_CALLBACK_RECEIVE:

		lwsl_user("LWS_CALLBACK_RECEIVE: %4d (rpp %5d, first %d, "
			  "last %d, bin %d, msglen %d (+ %d = %d))\n",
			  (int)len, (int)lws_remaining_packet_payload(wsi),
			  lws_is_first_fragment(wsi),
			  lws_is_final_fragment(wsi),
			  lws_frame_is_binary(wsi), session->msglen, (int)len,
			  (int)session->msglen + (int)len);

		if (len) {
			;
			// puts((const char *)in);
			// lwsl_hexdump_notice(in, len);
		}

		/* fs-ws-signal */
		struct fs_ws_signal_request s_request = fs_ws_signal_parse_request(in, len);
		struct fs_ws_signal_response s_response = fs_ws_signal_process(s_request);
		len = fs_ws_signal_get_response_size(s_response);
		char *stream = fs_ws_signal_serialize_response(s_response);
		/* -- */

		response.first = (char)lws_is_first_fragment(wsi);
		response.final = (char)lws_is_final_fragment(wsi);
		response.binary = (char)lws_frame_is_binary(wsi);
		n = (int)lws_ring_get_count_free_elements(session->ring);
		if (!n) {
			lwsl_user("dropping!\n");
			break;
		}

		if (response.final)
			session->msglen = 0;
		else
			session->msglen += (uint32_t)len;

		response.len = (uint32_t)len;
		/* notice we over-allocate by LWS_PRE */
		response.payload = malloc(LWS_PRE + len);
		if (!response.payload) {
			lwsl_user("OOM: dropping\n");
			break;
		}

		memcpy((char *)response.payload + LWS_PRE, stream, len);
		if (!lws_ring_insert(session->ring, &response, 1)) {
			__minimal_destroy_message(&response);
			lwsl_user("dropping!\n");
			break;
		}

		/* fs_ws_signal */
		free(stream);
		fs_ws_signal_free_response(s_response);
		fs_ws_signal_free_request(s_request);
		/* -- */

		lws_callback_on_writable(wsi);

		if (n < 3 && !session->flow_controlled) {
			session->flow_controlled = 1;
			lws_rx_flow_control(wsi, 0);
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
		callback_minimal_server_echo, \
		sizeof(struct per_session_data__minimal_server_echo), \
		1024, \
		0, NULL, 0 \
	}
