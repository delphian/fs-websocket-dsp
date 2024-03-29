/**
 * @file fs_ws_dsp_cmd_fft.c
 * @brief Fast fourier transform.
 */

void fs_ws_dsp_cmd_fft(struct fs_ws_dsp_command *command, struct fs_ws_dsp_message request, struct fs_ws_dsp_message *response) {
    uint32_t sample_rate = *((uint32_t *)command->params);
    uint32_t sample_size = *((uint32_t *)(command->params + 4));
    float complex *x;
    unsigned int n_len;
    int flags = 0;        // FFT flags (typically ignored)

    // Calculate number of samples.
    if (response->data == NULL) {
        // NO LIKE. SAMPLE RATE SHOULD RESPECT TYPE. IE X of COMPLEX SAMPLES (I AND Q AS A UNIT)
        n_len = request.data_len / sample_size / 2;
    } else {
        n_len = response->data_len / sizeof(float complex);
    }

    // create filter object, input and output samples.
    if (response->data == NULL) {
        x = fs_ws_dsp_samples_8to32(request.data, n_len);
    } else {
        x = response->data;
    }
    float complex *y = (float complex *) malloc(n_len * sizeof(float complex));

    // create FFT plan
    fftplan q = fft_create_plan(n_len, x, y, LIQUID_FFT_FORWARD, flags);
    fft_execute(q);

    // destroy FFT plan and free memory arrays
    fft_destroy_plan(q);
    free(x); // (response->data)

    response->_version       = 1;
    response->id             = request.id;
    response->commands_count = 0;
    response->commands       = NULL;
    response->data_len       = n_len * sizeof(float complex);
    response->data           = (char *)y;

    return;
}
