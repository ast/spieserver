//
//  dsp.c
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#include "dsp.h"

#include <complex.h>
#include <math.h>
#include <assert.h>
#include <volk/volk.h>

// point n of an 0 to M (M+1 points)
float hamming(int n, int M) {
    return 0.54-0.46*cosf(2.*M_PI*n/M);
}

// point n of an 0 to M (M+1 points)
float blackman(int n, int M) {
    return 0.42-0.5*cosf(2*M_PI*n/M)+0.08*cosf(4.*M_PI*n/M);
}

void rotate_pi(float *in, int len) {
    int s = 1;
    for (int n = 0; n < len; n++) {
        in[n] *= s;
        s *= -1;
    }
}

void window(float *win, int len) {
    int M = len - 1;
    for (int n = 0; n <= M; n++) {
        win[n] = hamming(n, M);
    }
}

void normalize(const float *in, float *out, int len) {
    float sum = 0;
    for (int i = 0; i < len; i++) {
        sum += in[i];
        out[i] = in[i];
    }
    volk_32f_s32f_normalize(out, sum, len);
}

// len must be odd
void sinc(float *h, float fc, int len) {
    int M = len - 1;
    assert(M % 2 == 0);
    for (int i = 0; i <= M; i++) {
        if (i == M/2) {
            // to avoid divide by zero
            h[i] = 2. * M_PI * fc;
            // the window is 1 here.
        } else {
            h[i] = sinf(2.*M_PI*fc*(i-M/2))/(i-M/2);
            h[i] *= hamming(i, M); // Window
        }
    }
    // Normalize for unity gain
    normalize(h, h, len);
}

void fold(const float complex* in, float complex *out, const float scale, int fold, int len) {
    const int N = len/fold;
    for (int offset = N; offset < len; offset+=N) {
        volk_32fc_x2_add_32fc(out, out, &in[offset], N);
    }
    volk_32f_s32f_multiply_32f((float*)out, (float*)out, scale, 2*N);
}
