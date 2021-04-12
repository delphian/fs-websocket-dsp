/**
 * @file fs_ws_dsp.c
 * @brief Websocket server subprotocol for digital signal processing requests
 */

#include <complex.h>
#include <unistd.h>
#include <liquid/liquid.h>

#include "fs_ws_dsp_command.c"
#include "fs_ws_dsp_fn_echo.c"

struct fs_ws_dsp_response fs_ws_dsp_process(struct fs_ws_dsp_request request) {
    struct fs_ws_dsp_response response;
    memset(&response, 0, sizeof(struct fs_ws_dsp_response));

// ROUTE REQUEST

    return response;
}
void fs_ws_dsp_free_response(struct fs_ws_dsp_response response) {
	if (response.data != NULL)
		free(response.data);
    memset(&response, 0, sizeof(struct fs_ws_dsp_response));
    return;
}
struct fs_ws_dsp_request fs_ws_dsp_parse_request(char *data, size_t data_len) {
    struct fs_ws_dsp_request request;
    memset(&request, 0, sizeof(struct fs_ws_dsp_request));
    char *src = data;
    request._version       = *((uint8_t *)src);     src += 1;
    request.id             = *((uint32_t *)src);    src += 4;
    request.commands_count = *((uint32_t *)src);    src += 4;
    if (request.commands_count > 0) {
        request.commands = malloc(sizeof(struct fs_ws_dsp_command *) * request.commands_count);
        struct fs_ws_dsp_command **dst = request.commands;
        for (int i = 0; i < request.commands_count; i++) {
            struct fs_ws_dsp_command *command = fs_ws_dsp_command_parse(src);
            *dst++ = command;
            src += fs_ws_dsp_command_serialize_size(command);
        }
    }
    request.data_len = *((uint32_t *)src);          src += 4;
    if (request.data_len > 0) {
        request.data = malloc(request.data_len);
        memcpy(request.data, src, request.data_len);
    }
    return request;
}
void fs_ws_dsp_free_request(struct fs_ws_dsp_request request) {
	if (request.data != NULL)
		free(request.data);
    if (request.commands_count > 0) {
        struct fs_ws_dsp_command **command = request.commands;
        for (int i = 0; i < request.commands_count; i++)
            fs_ws_dsp_command_free(*(command++));
        free (request.commands);
    }
    memset(&request, 0, sizeof(struct fs_ws_dsp_request));
    return;
}
char *fs_ws_dsp_serialize_response(struct fs_ws_dsp_response response) {
    char *stream;
    char *dest;
    stream = malloc(fs_ws_dsp_get_response_size(response));
    dest = stream;
    memcpy(dest,                             &response._version, sizeof response._version);
    memcpy(dest += sizeof response._version, &response.id,       sizeof response.id);
    memcpy(dest += sizeof response.id,       &response.data_len, sizeof response.data_len);
    memcpy(dest += sizeof response.data_len, response.data,      response.data_len);
    return stream;
}
size_t fs_ws_dsp_get_response_size(struct fs_ws_dsp_response response) {
    return sizeof response._version +
           sizeof response.id + 
           sizeof response.data_len +
           response.data_len;
}
