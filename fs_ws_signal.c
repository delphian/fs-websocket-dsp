#include <complex.h>
#include <liquid/liquid.h>

struct fs_ws_signal_response fs_ws_signal_process(struct fs_ws_signal_request request) {
    struct fs_ws_signal_response response;
    size_t data_len = 4;
    response._version = 1;
    response.id = request.id;
    response.data = malloc(data_len);
    response.data[0] = 'a';
    response.data[1] = 'b';
    response.data[2] = 'c';
    response.data[3] = 'd';
    response.data_len = data_len;
    return response;
}
void fs_ws_signal_free_response(struct fs_ws_signal_response response) {
	if (response.data != NULL)
		free(response.data);
    memset(&response, 0, sizeof(struct fs_ws_signal_response));
    return;
}
struct fs_ws_signal_request fs_ws_signal_parse_request(char *data, size_t data_len) {
    struct fs_ws_signal_request request;
    request._version = 1;
    request.id = 257;
    request.data = NULL;
    request.data_len = 0;
    return request;
}
void fs_ws_signal_free_request(struct fs_ws_signal_request request) {
	if (request.data != NULL)
		free(request.data);
    memset(&request, 0, sizeof(struct fs_ws_signal_request));
    return;
}
char *fs_ws_signal_serialize_response(struct fs_ws_signal_response response) {
    char *stream;
    char *dest;
    stream = malloc(fs_ws_signal_get_response_size(response));
    dest = stream;
    memcpy(dest, &response._version, sizeof response._version);
    memcpy(dest += sizeof response._version, &response.id, sizeof response.id);
    memcpy(dest += sizeof response.id, response.data, response.data_len);
    return stream;
}
size_t fs_ws_signal_get_response_size(struct fs_ws_signal_response response) {
    return sizeof response._version +
           sizeof response.id + 
           response.data_len;
}
