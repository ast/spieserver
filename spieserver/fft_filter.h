//
//  fft_filter.h
//  spieserver
//
//  Created by Albin Stigö on 2019-12-13.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#ifndef fft_filter_h
#define fft_filter_h

#include <stdio.h>
#include <complex.h>
#include <fftw3.h>
#include <math.h>

/*
 *
 * P = 801
 * L = 2048
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
} fft_filter_t;

int fft_filt_init(fft_filter_t *ff, int h_len, int block_len, int dec, int rotate);
void fft_filt_free(fft_filter_t *ff);
int fft_filt_execute(fft_filter_t *ff, float complex *in, float complex *out);
void fft_filt_set_proto(fft_filter_t *ff, float fc);
void fft_filt_set_center(fft_filter_t *ff, float fc);

//void fft_filt_print_freq_resp(fft_filter_t *ff);


#endif /* fft_filter_h */
