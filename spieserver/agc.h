//
//  agc.h
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#ifndef agc_h
#define agc_h

#include <stdio.h>
#include <complex.h>
#include <math.h>

typedef struct {
    float atack;
    float decay;
    float gain;
    float max_gain;
} agc_t;

void agc_init(agc_t *a, float attack, float decay);
void agc_process(agc_t *a, float complex *in, float complex *out, int L);

#endif /* agc_h */
