//
//  spectrum.h
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#ifndef spectrum_h
#define spectrum_h

#include <stdio.h>
#include <pthread.h>
#include <complex.h>
#include <fftw3.h>
#include "ringbuffer.h"

typedef struct {
    pthread_t thread;
    pthread_mutex_t mutex;
    size_t          len;
    float          *win;
    float          *psd;
    ringbuffer_t   *rb;
    fftwf_plan     fft_fwd_plan;
    fftwf_complex  *fft_in;
    fftwf_complex  *fft_out;
} spectrum_t;

spectrum_t* spec_create(size_t len);
int spec_start(spectrum_t *spec);
void spec_stop(spectrum_t *spec);
void spec_destroy(spectrum_t *spec);
void spec_add_buffer(spectrum_t *spec, float complex *in, size_t len);

#endif /* spectrum_h */
