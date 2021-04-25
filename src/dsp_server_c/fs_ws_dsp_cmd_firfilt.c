/**
 * @file fs_ws_dsp_cmd_firfilt.c
 * @brief FIR Filter.
 */

void fs_ws_dsp_cmd_firfilt(struct fs_ws_dsp_command *command, struct fs_ws_dsp_message request, struct fs_ws_dsp_message *response) {
    uint32_t sample_rate = *((uint32_t *)command->params);
    uint32_t sample_size = *((uint32_t *)(command->params + 4));
    float complex *x;
    unsigned int n_len;

    // Calculate number of samples.
    if (response->data == NULL) {
        // NO LIKE. SAMPLE RATE SHOULD RESPECT TYPE. IE X of COMPLEX SAMPLES (I AND Q AS A UNIT)
        n_len = request.data_len / sample_size / 2;
    } else {
        n_len = response->data_len / sizeof(float complex);
    }

    // Construct filter coefficients:
    unsigned int h_len=57;      // filter length
    float fc=0.10f;             // cutoff frequency
    float As=60.0f;             // stop-band attenuation
    float h[h_len];
    liquid_firdes_kaiser(h_len, fc, As, 0, h);

    // create filter object, input and output samples.
    firfilt_crcf q = firfilt_crcf_create(h, h_len);
    if (response->data == NULL) {
        x = fs_ws_dsp_samples_8to32(request.data, n_len);
    } else {
        x = response->data;
    }
    float complex *y = (float complex *) malloc(n_len * sizeof(float complex));

    // Run signal through filter one sample at a time.
    int i;
    for (i = 0; i < n_len; i++) {
        firfilt_crcf_push(q, x[i]);
        firfilt_crcf_execute(q, &y[i]);
    }

    // destroy filter object
    firfilt_crcf_destroy(q);
    free(x);

    response->_version       = 1;
    response->id             = request.id;
    response->commands_count = 0;
    response->commands       = NULL;
    response->data_len       = n_len * sizeof(float complex);
    response->data           = (char *)y;

    return;
}
