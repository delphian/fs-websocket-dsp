/**
 * @file fs_ws_dsp.c
 * @brief Websocket server subprotocol for digital signal processing requests
 */

#include <complex.h>
#include <liquid/liquid.h>

struct fs_ws_dsp_response fs_ws_dsp_process(struct fs_ws_dsp_request request) {
    struct fs_ws_dsp_response response;
    size_t data_len = 4;
    response._version = 1;
    response.id = request.id;
    response.data = malloc(data_len);
    *(uint32_t *)response.data = request.data_len;
    response.data_len = data_len;
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
    request._version = (uint8_t)data[0];
    request.id       = *((uint32_t *)(data + 1));
    request.data_len = *((uint32_t *)(data + 5));
    request.data     = malloc(request.data_len);
                       memcpy(request.data, data + 9, request.data_len);
    return request;
}
void fs_ws_dsp_free_request(struct fs_ws_dsp_request request) {
	if (request.data != NULL)
		free(request.data);
    memset(&request, 0, sizeof(struct fs_ws_dsp_request));
    return;
}
char *fs_ws_dsp_serialize_response(struct fs_ws_dsp_response response) {
    char *stream;
    char *dest;
    stream = malloc(fs_ws_dsp_get_response_size(response));
    dest = stream;
    memcpy(dest, &response._version, sizeof response._version);
    memcpy(dest += sizeof response._version, &response.id, sizeof response.id);
    memcpy(dest += sizeof response.id, response.data, response.data_len);
    return stream;
}
size_t fs_ws_dsp_get_response_size(struct fs_ws_dsp_response response) {
    return sizeof response._version +
           sizeof response.id + 
           response.data_len;
}
