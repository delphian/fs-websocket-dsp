/**
 * @file fs_ws_dsp.h
 * @brief Websocket server subprotocol for digital signal processing messages
 */

#include "fs_ws_dsp_command.h"
#include "fs_ws_dsp_message.h"

/**
 * @brief Process signal processing message.
 * @param[in] message Signal processing message.
 */
struct fs_ws_dsp_message fs_ws_dsp_process(struct fs_ws_dsp_message message);
