//
//  fft_xlating.c
//  spieserver
//
//  Created by Albin Stigö on 2019-12-13.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#include "fft_xlating.h"

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

/*void fft_filt_print_freq_resp(fft_xlating_t *ff); {
 // TODO: fix PSD
 for (int i = 0; i < N; i++) {
 float db = 20. * logf(cabsf(fx->H[i]));
 printf("%f\n", db);
 }
 }*/

int fft_xlate_init(fft_xlating_t *fx, int h_len, int block_len, int dec) {
    // These conditions are necessary
    assert(h_len % 2 == 1);
    assert((h_len - 1) % dec == 0);
    assert((block_len + h_len - 1) % dec == 0);
    
    const int N = block_len + h_len - 1;
    printf("%d\n", N);
    fx->h_len = h_len;
    fx->block_len = block_len;
    fx->dec = dec;
    
    // Signal forward transform
    fx->in = fftwf_alloc_complex(N);
    fx->out = fftwf_alloc_complex(N);
    assert(fx->in != NULL && fx->out != NULL);
    fx->fwd_plan = fftwf_plan_dft_1d(N, fx->in, fx->out, FFTW_FORWARD, FFTW_MEASURE);
    
    // Signal inverse transform
    fx->in_dec = fftwf_alloc_complex(N/dec);
    fx->out_dec = fftwf_alloc_complex(N/dec);
    assert(fx->in_dec != NULL && fx->out_dec != NULL);
    fx->inv_plan = fftwf_plan_dft_1d(N/dec, fx->in_dec, fx->out_dec, FFTW_BACKWARD, FFTW_MEASURE);
    
    // Filter impulse response forward transform
    // TODO: change to volk_malloc
    fx->h_pro = malloc(h_len*sizeof(float));
    fx->h = fftwf_alloc_complex(N);
    fx->H = fftwf_alloc_complex(N);
    fx->fwd_filter_plan = fftwf_plan_dft_1d(N, fx->h, fx->H, FFTW_FORWARD, FFTW_MEASURE);
    
    return 0;
}

void fft_xlate_free(fft_xlating_t *fx) {
    fftwf_free(fx->in);
    fftwf_free(fx->out);
    fftwf_destroy_plan(fx->fwd_plan);
    
    fftwf_free(fx->in_dec);
    fftwf_free(fx->out_dec);
    fftwf_destroy_plan(fx->inv_plan);
    
    free(fx->h_pro);
    fftwf_free(fx->h);
    fftwf_free(fx->H);
    fftwf_destroy_plan(fx->fwd_filter_plan);
}

void fft_xlate_set_proto(fft_xlating_t *fx, float fc) {
    // Create simple prototype filter
    sinc(fx->h_pro, fc, fx->h_len);
}

void fft_xlate_set_center(fft_xlating_t *fx, float fc) {
    const int N = fx->block_len + fx->h_len - 1;
    float complex w = I * 2. * M_PI * fc;
    // Copy prototype to fft input and rotate to fc
    // Decimation will then alias down the bandpass signal
    for (int n = 0; n < fx->h_len; n++) {
        fx->h[n] = fx->h_pro[n] * cexpf(w * n);
    }
    // Zero the rest (this is padding for the FFT)
    for (int n = fx->h_len; n < N; n++) {
        fx->h[n] = 0;
    }
    // Calculate frequency response so we can filter in the
    // frequency domain
    fftwf_execute(fx->fwd_filter_plan);
    // Phase increment and current phase
    // for final adjustment applied at decimated rate.
    fx->w = cexpf(-w * fx->dec);
    fx->p = 1.;
}

int fft_xlate_execute(fft_xlating_t *fx, float complex *in, float complex *out) {
    const int N = fx->block_len + fx->h_len - 1;
    // Move tail of buffer back to beginning, we need this overlap
    // because we are doing circular convolution
    memcpy(&fx->in[0], &fx->in[fx->block_len], (fx->h_len-1)*sizeof(float complex));
    // Copy in new samples
    memcpy(&fx->in[fx->h_len-1], in, fx->block_len*sizeof(float complex));
    // Perform forward FFT
    fftwf_execute(fx->fwd_plan);
    // Circular convolution in the frequency domain
    volk_32fc_x2_multiply_32fc(fx->out, fx->out, fx->H, N);
    
    // TODO: fix scale
    float scale = (1/32.) * (1.f/(float)fx->dec);
    fold(fx->out, fx->in_dec, scale, fx->dec, N);
    
    // Execute inverse transform
    fftwf_execute(fx->inv_plan);
    // Apply final rotation to center at 0Hz.
    // Notice that we discard P-1 samples because of the circular convolution.
    volk_32fc_s32fc_x2_rotator_32fc(out,
                                    &fx->out_dec[(fx->h_len-1)/fx->dec],
                                    fx->w,
                                    &fx->p,
                                    (fx->block_len/fx->dec));
    
    return 0;
}
