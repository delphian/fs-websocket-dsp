/**
 * @file fs_ws_dsp.c
 * @brief Websocket server subprotocol for digital signal processing requests
 */

#include <liquid/liquid.h>

struct fs_ws_dsp_message fs_ws_dsp_process(struct fs_ws_dsp_message request) {
    struct fs_ws_dsp_message response;
    memset(&response, 0, sizeof(struct fs_ws_dsp_message));

    // ROUTE REQUEST

    return response;
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