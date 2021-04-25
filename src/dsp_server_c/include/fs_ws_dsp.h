/**
 * @file fs_ws_dsp.h
 * @brief Websocket server subprotocol for digital signal processing messages
 */

#include "fs_ws_dsp_command.h"
#include "fs_ws_dsp_message.h"
#include "fs_ws_dsp_cmd_echo.h"
#include "fs_ws_dsp_cmd_fft.h"
#include "fs_ws_dsp_cmd_firfilt.h"

/**
 * @brief Process signal processing message.
 * @param[in] message Signal processing message.
 */
struct fs_ws_dsp_message fs_ws_dsp_process(struct fs_ws_dsp_message message);
/**
 * @brief Convert interleaved complex char (8 bit I, 8 bit Q) samples to interleaved float complex (32 bit I, 32 bit Q) samples
 * @param[in] samples Interleaved complex char samples
 * @param[in] n_samples Number of samples (IQ pairs)
 */
float _Complex *fs_ws_dsp_samples_8to32(char _Complex *samples, int n_samples);

void fs_ws_dsp_debug(char *data, size_t data_len);
