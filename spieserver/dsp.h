//
//  dsp.h
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#ifndef dsp_h
#define dsp_h

#include <stdio.h>
#include <complex.h>


float hamming(int n, int M);
float blackman(int n, int M);
void rotate_pi(float *in, int len);
void window(float *win, int len);
void normalize(const float *in, float *out, int len);
void sinc(float *h, float fc, int len);
void fold(const float complex* in, float complex *out, const float scale, int fold, int len);
//void fold_2848_32(const float complex* in, float complex *out);

#endif /* dsp_h */
