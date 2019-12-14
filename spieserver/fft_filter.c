//
//  fft_filter.c
//  spieserver
//
//  Created by Albin Stigö on 2019-12-13.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#include "fft_filter.h"

#include "dsp.h"
#include <fftw3.h>
#include <complex.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <volk/volk.h>

/*
 * N = L + P - 1
 * P − 1 = K1 * D
 * L + P − 1 = K2 * D
 *
 * K1 och K2 has to be integers
 *
 */

/*void fft_filt_print_freq_resp(fft_filter_t *ff); {
 // TODO: fix PSD
 for (int i = 0; i < N; i++) {
 float db = 20. * logf(cabsf(ff->H[i]));
 printf("%f\n", db);
 }
 }*/

int fft_filt_init(fft_filter_t *ff, int h_len, int block_len, int dec, int rotate) {
    // These conditions are necessary
    assert(h_len % 2 == 1);
    assert((h_len - 1) % dec == 0);
    assert((block_len + h_len - 1) % dec == 0);
    
    const int N = block_len + h_len - 1;
    printf("%d\n", N);
    ff->h_len = h_len;
    ff->block_len = block_len;
    ff->dec = dec;
    
    // Signal forward transform
    ff->in = fftwf_alloc_complex(N);
    ff->out = fftwf_alloc_complex(N);
    assert(ff->in != NULL && ff->out != NULL);
    ff->fwd_plan = fftwf_plan_dft_1d(N, ff->in, ff->out, FFTW_FORWARD, FFTW_MEASURE);
    
    // Signal inverse transform
    ff->in_dec = fftwf_alloc_complex(N/dec);
    ff->out_dec = fftwf_alloc_complex(N/dec);
    assert(ff->in_dec != NULL && ff->out_dec != NULL);
    ff->inv_plan = fftwf_plan_dft_1d(N/dec, ff->in_dec, ff->out_dec, FFTW_BACKWARD, FFTW_MEASURE);
    
    // Filter impulse response forward transform
    // TODO: change to volk_malloc
    ff->h_pro = malloc(h_len*sizeof(float));
    ff->h = fftwf_alloc_complex(N);
    ff->H = fftwf_alloc_complex(N);
    ff->fwd_filter_plan = fftwf_plan_dft_1d(N, ff->h, ff->H, FFTW_FORWARD, FFTW_MEASURE);
    
    return 0;
}

void fft_filt_free(fft_filter_t *ff) {
    fftwf_free(ff->in);
    fftwf_free(ff->out);
    fftwf_destroy_plan(ff->fwd_plan);
    
    fftwf_free(ff->in_dec);
    fftwf_free(ff->out_dec);
    fftwf_destroy_plan(ff->inv_plan);
    
    free(ff->h_pro);
    fftwf_free(ff->h);
    fftwf_free(ff->H);
    fftwf_destroy_plan(ff->fwd_filter_plan);
}

void fft_filt_set_proto(fft_filter_t *ff, float fc) {
    // Create simple windowed sinc prototype filter
    sinc(ff->h_pro, fc, ff->h_len);
}

void fft_filt_set_center(fft_filter_t *ff, float fc) {
    const int N = ff->block_len + ff->h_len - 1;
    float complex w = I * 2. * M_PI * fc;
    // Copy prototype to FFT input and rotate to fc
    for (int n = 0; n < ff->h_len; n++) {
        ff->h[n] = ff->h_pro[n] * cexpf(w * n);
    }
    // Zero the rest (this is padding for the FFT)
    for (int n = ff->h_len; n < N; n++) {
        ff->h[n] = 0;
    }
    // Calculate frequency response so we can filter in the
    // frequency domain
    fftwf_execute(ff->fwd_filter_plan);
}

int fft_filt_execute(fft_filter_t *ff, float complex *in, float complex *out) {
    const int N = ff->block_len + ff->h_len - 1;
    // Move tail of buffer back to beginning, we need this overlap
    // because we are doing circular convolution
    memcpy(&ff->in[0], &ff->in[ff->block_len], (ff->h_len-1)*sizeof(float complex));
    // Copy in new samples
    memcpy(&ff->in[ff->h_len-1], in, ff->block_len*sizeof(float complex));
    // Perform forward FFT
    fftwf_execute(ff->fwd_plan);
    // Circular convolution in the frequency domain
    volk_32fc_x2_multiply_32fc(ff->out, ff->out, ff->H, N);
    // TODO: Optimize decimation in the frequency domain
    
    if (ff->dec > 1) {
        float scale = (1/32.) * (89/2848.);
        fold(ff->out, ff->in_dec, scale, ff->dec, N);
    } else {
        float scale = (89/2848.);
        volk_32f_s32f_multiply_32f((float*)ff->in_dec, (float*)ff->out, scale, 2*N);
    }
    
    // Execute inverse transform
    fftwf_execute(ff->inv_plan);
    // Notice that we discard P-1 samples because of the circular convolution.
    memcpy(out, &ff->out_dec[(ff->h_len-1)/ff->dec],
           (ff->block_len/ff->dec)*sizeof(float complex));
    
    return 0;
}
