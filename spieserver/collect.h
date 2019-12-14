//
//  collect.h
//  spieserver
//
//  Created by Albin Stigö on 2019-12-14.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#ifndef collect_h
#define collect_h

#include <stdio.h>
#include <complex.h>

typedef struct  {
    
} collect_t;

int collect_init(size_t block_size, int num_blocks);

static inline float complex *collect_in(collect_t* co) {
    return NULL;
}

static inline float complex *collect_out(collect_t* co) {
    return NULL;
}

#endif /* collect_h */
