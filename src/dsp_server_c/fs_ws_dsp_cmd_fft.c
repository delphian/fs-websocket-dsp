/**
 * @file fs_ws_dsp_cmd_fft.c
 * @brief Fast fourier transform.
 */

void fs_ws_dsp_cmd_fft(struct fs_ws_dsp_command *command, struct fs_ws_dsp_message request, struct fs_ws_dsp_message *response) {
    uint32_t sample_rate = *((uint32_t *)command->params);
    uint32_t sample_size = *((uint32_t *)(command->params + 4));

    // NO LIKE. SAMPLE RATE SHOULD RESPECT TYPE. IE X of COMPLEX SAMPLES (I AND Q AS A UNIT)
    unsigned int n = request.data_len / sample_size / 2;  // input data size
    int flags = 0;        // FFT flags (typically ignored)
    // allocated memory arrays
    float complex *x = (float complex*) malloc(n * sizeof(float complex));
    float complex *y = (float complex*) malloc(n * sizeof(float complex));

    // create FFT plan
    fftplan q = fft_create_plan(n, x, y, LIQUID_FFT_FORWARD, flags);

    // ... initialize input ...
    int i;
    char complex *src = request.data;
    float complex *dst = x;
    for (i = 0; i < n; i++) {
        *dst++ = (float complex)*src++; 
    }

    // execute FFT (repeat as necessary)
    fft_execute(q);

    // destroy FFT plan and free memory arrays
    fft_destroy_plan(q);
    free(x);
    // free(y);

    response->_version       = 1;
    response->id             = request.id;
    response->commands_count = 0;
    response->commands       = NULL;
    response->data_len       = n * sizeof(float complex);
    response->data           = (char *)y;

    return;
}
