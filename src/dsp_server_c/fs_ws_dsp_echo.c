/**
 * @file fs_ws_dsp_echo.c
 * @brief Echo payload back at client.
 */

void fs_ws_dsp_process_echo(struct fs_ws_dsp_request request, struct fs_ws_dsp_response *response) {
    response->_version = 1;
    response->id       = request.id;
    response->data_len = request.data_len;
    if (response->data_len > 0) {
        response->data = malloc(response->data_len);
        memcpy(response->data, request.data, request.data_len);
    }
    return;
}
