//
//  fir.h
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#ifndef fir_h
#define fir_h

#include <stdio.h>
#include <complex.h>
#include <volk/volk.h>
#include <assert.h>
#include <string.h>

#include "ringbuffer.h"
#include "dsp.h"

typedef struct {
    int P; // length of h
    int L; // length of block
    int D; // decimation
    float *h;
    float complex *z;
    ringbuffer_t *rb;
} fir_t;


int fir_new(fir_t *f, int P, int L, int D);
void fir_free(fir_t *f);
int fir_filter(fir_t *f, float complex *in, float complex *out);
void fir_set_fc(fir_t *f, float fc);


#endif /* fir_h */
