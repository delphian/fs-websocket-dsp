/**
 * @file fs_ws_dsp.c
 * @brief Websocket server subprotocol for digital signal processing requests
 */

#include <liquid/liquid.h>

#include "fs_ws_dsp_command.c"
#include "fs_ws_dsp_message.c"
#include "fs_ws_dsp_cmd_echo.c"
#include "fs_ws_dsp_cmd_fft.c"

struct fs_ws_dsp_message fs_ws_dsp_process(struct fs_ws_dsp_message request) {
    struct fs_ws_dsp_message response;
    memset(&response, 0, sizeof(struct fs_ws_dsp_message));
    response.id = request.id;
    if (request.commands_count > 0) {
        struct fs_ws_dsp_command **dst = request.commands;
        for (int i = 0; i < request.commands_count; i++) {
            struct fs_ws_dsp_command *command = *dst++;            
            if (command->type == FS_WS_DSP_CMD_ECHO)
                fs_ws_dsp_cmd_echo(command, request, &response);
            if (command->type == FS_WS_DSP_CMD_FFT)
                fs_ws_dsp_cmd_fft(command, request, &response);
        }
    }
    return response;
}

void fs_ws_dsp_debug(char *data, size_t data_len) {
    int i;
    fprintf(stderr, "\n");
    for (i = 0; i < data_len; i++) {
        fprintf(stderr, "%03u.", (uint8_t)data[i]);
        if (i%8 == 0)
            fprintf(stderr, "\n");
    }
    return;
}

int interpolator() {
    unsigned int M  = 4;     // interpolation factor
    unsigned int m  = 12;    // filter delay [symbols]
    float        As = 60.0f; // filter stop-band attenuation [dB]

    // create interpolator from prototype
    firinterp_crcf interp = firinterp_crcf_create_kaiser(M,m,As);
    float complex x = 1.0f + 0.5f * I;  // input sample
    float complex y[M];      // interpolated output buffer

    // repeat on input sample data as needed
    {
        firinterp_crcf_execute(interp, x, y);
    }

    // destroy interpolator object
    firinterp_crcf_destroy(interp);

	int i;
	for (i = 0; i < M; i++) {
		printf("%f + i%f\n", creal(y[i]), cimag(y[i]));
	}

    return 0;
}