//
//  fir.c
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#include "fir.h"

void fir_set_fc(fir_t *f, float fc) {
    // TODO: implement better filters
    sinc(f->h, fc, f->P);
}

int fir_new(fir_t *f, int P, int L, int D) {
    f->P = P; // h len
    f->L = L; // block size
    f->D = D; // decimation
    f->h = volk_malloc(P*sizeof(float), 32);
    assert(f->h != NULL);
    // Will get rounded up to a multiple of pagesize
    f->rb = rb_create(L + P - 1);
    // Inital zeros
    rb_produce(f->rb, (P-1)*sizeof(float complex));
    
    return 0;
}

int fir_filter(fir_t *f, float complex *in, float complex *out) {
    // Copy input to ringbuffer
    size_t blk_size = f->L*sizeof(float complex);
    // Copy input to delay buffer
    memcpy(rb_writeptr(f->rb), in, blk_size);
    rb_produce(f->rb, blk_size);
    // Start of delay line
    float complex *z = (float complex*)rb_readptr(f->rb);
    // Convolve
    for (int n = 0; n < f->L; n++) {
        volk_32fc_32f_dot_prod_32fc(&out[n], &z[n], f->h, f->P);
    }
    // Consume L items from delay leaving a P-1 tail.
    rb_consume(f->rb, blk_size);
    
    return 0;
}

void fir_free(fir_t *f) {
    volk_free(f->h);
}
