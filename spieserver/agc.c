//
//  agc.c
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#include "agc.h"

void agc_init(agc_t *a, float attack, float decay) {
    a->atack = attack;
    a->decay = decay;
    a->gain = 1.0;
}

void agc_process(agc_t *a, float complex *in, float complex *out, int L) {
    const float target = M_SQRT1_2;
    
    for (int n = 0; n < L; n++) {
        float y = in[n] * a->gain;
        // result is real
        float pow = crealf(y * conjf(y));
        if (pow > target) {
            a->gain -= a->gain * a->decay;
        } else {
            a->gain += a->gain * a->atack;
        }
        out[n] = y;
    }
}
