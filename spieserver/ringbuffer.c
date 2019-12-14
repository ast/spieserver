//
//  ringbuffer.c
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#include "ringbuffer.h"

#include "doublemap.h"
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

// Round up to page size
size_t roundup_mult_pagesize(size_t size) {
    size_t pagesize =getpagesize();
    return size + pagesize - 1 - (size - 1) % pagesize;
}

// Create
ringbuffer_t* rb_create(size_t size) {
    size = roundup_mult_pagesize(size);

    ringbuffer_t *rb = malloc(sizeof(ringbuffer_t));
    assert(rb != NULL);
    rb->buffer = doublemap(size);
    assert(rb->buffer != NULL);
    rb->read = 0;
    rb->write = 0;
    rb->size = size;
    return rb;
}

void rb_free(ringbuffer_t *rb) {
    doublemunlock(rb->buffer, rb->size);
    free(rb);
}
