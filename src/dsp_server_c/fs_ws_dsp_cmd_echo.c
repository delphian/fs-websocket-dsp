/**
 * @file fs_ws_dsp_cmd_echo.c
 * @brief Echo payload back at client.
 */

void fs_ws_dsp_process_echo(struct fs_ws_dsp_message request, struct fs_ws_dsp_message *response) {
    response->_version       = 1;
    response->id             = request.id;
    response->data_len       = request.data_len;
    response->commands_count = 0;
    // WE NEED A FUNCTION TO ADD COMMANDS TO A MESSAGE.
    if (response->data_len > 0) {
        response->data = malloc(response->data_len);
        memcpy(response->data, request.data, request.data_len);
    }
    return;
}
