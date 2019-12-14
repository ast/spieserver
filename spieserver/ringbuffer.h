//
//  ringbuffer.h
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#ifndef ringbuffer_h
#define ringbuffer_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
/*
 * Inspired by:
 * https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
 * https://github.com/willemt/cbuffer/
 * https://github.com/michaeltyson/TPCircularBuffer
 */

typedef struct {
    void    *buffer;
    size_t  read;
    size_t  write;
    // size MUST be power of two!!
    size_t  size;
} ringbuffer_t;

// Creation
ringbuffer_t* rb_create(size_t size);
void cb_free(ringbuffer_t *rb);

#define RB_MASK(rb, val) (val & (rb->size - 1))

// Access
static inline void rb_produce(ringbuffer_t *rb, size_t amount)
{
    rb->write += amount;
}

static inline void rb_consume(ringbuffer_t *rb, size_t amount)
{
    rb->read += amount;
}

// Pointer to head ready for writing
static inline void* rb_writeptr(ringbuffer_t *rb)
{
    return &rb->buffer[RB_MASK(rb, rb->write)];
}

static inline void rb_memcpy(ringbuffer_t *rb, const void *src, size_t n)
{
    void *dst = rb_writeptr(rb);
    memcpy(dst, src, n);
    rb_produce(rb, n);
}

// Pointer to head ready for writing
static inline void* rb_writeptr_avail(ringbuffer_t *rb, size_t *avail)
{
    // available for writing
    *avail = rb->write - rb->read;
    return &rb->buffer[RB_MASK(rb, rb->write)];
}


// Pointer to tail, ready for reading
static inline void* rb_readptr(ringbuffer_t *rb)
{
    return &rb->buffer[RB_MASK(rb, rb->read)];
}

// Get a pointer hist bytes behind writeptr
static inline void* rb_histptr(ringbuffer_t *rb, size_t hist)
{
    return &rb->buffer[RB_MASK(rb, rb->write - hist)];
}

// Pointer to tail, ready for reading
static inline void* rb_readptr_avail(ringbuffer_t *rb, size_t *avail)
{
    // available for reading
    *avail = rb->size - (rb->write - rb->read);
    return &rb->buffer[RB_MASK(rb, rb->read)];
}

#endif /* ringbuffer_h */
