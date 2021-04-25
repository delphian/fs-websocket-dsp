/**
 * @file fs_ws_dsp_cmd_firfilt.h
 * @brief FIR Filter.
 */

/**
 * @brief Fir Filter.
 * @param[in]     command  - Processing request command details.
 * @param[in]     request  - Full request from client.
 * @param[in out] response - Response to be sent back to client. May contain data from previous processing command.
 */
void fs_ws_dsp_cmd_firfilt(struct fs_ws_dsp_command *command, struct fs_ws_dsp_message request, struct fs_ws_dsp_message *response);
