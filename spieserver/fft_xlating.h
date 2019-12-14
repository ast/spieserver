//
//  fft_xlating.h
//  spieserver
//
//  Created by Albin Stigö on 2019-12-13.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#ifndef fft_xlating_h
#define fft_xlating_h

#include <stdio.h>
#include <complex.h>
#include <fftw3.h>
#include <math.h>

/*
 *
 * P = ...
 * L = ...
 * N = L + P - 1
 * P − 1 = K1 * D
 * L + P − 1 = K2 * D
 *
 * K1 och K2 has to be integers
 *
 */

typedef struct {
    int            h_len;
    int            block_len;
    int            dec;
    
    float complex  w; // Phase increment for frequency correction after decimation
    float complex  p; // Current phase for correction
    
    fftwf_plan     fwd_plan; // First FFT forward plan
    fftwf_complex  *in; // FFT in
    fftwf_complex  *out; // FFT out
    
    fftwf_plan    inv_plan; // First FFT inverse plan (decimated)
    fftwf_complex *in_dec;  // FFT in decimated buffer
    fftwf_complex *out_dec; // FFT out decimated buffer
    
    fftwf_plan    fwd_filter_plan; // FFT plan for filter
    float         *h_pro; // Filter prototype
    fftwf_complex *h; // Filter impulse response
    fftwf_complex *H; // Filter impulse response frequency domain
} fft_xlating_t;

int fft_xlate_init(fft_xlating_t *fx, int h_len, int block_len, int dec);
void fft_xlate_free(fft_xlating_t *fx);
int fft_xlate_execute(fft_xlating_t *fx, float complex *in, float complex *out);
void fft_xlate_set_proto(fft_xlating_t *fx, float fc);
void fft_xlate_set_center(fft_xlating_t *fx, float fc);
//void fft_filt_print_freq_resp(fft_filter_t *ff);


#endif /* fft_xlating_h */
