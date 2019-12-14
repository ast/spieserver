//
//  spectrum.c
//  spieserver
//
//  Created by Albin Stigö on 2019-12-08.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#include "spectrum.h"

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <complex.h>
#include <pthread.h>
#include <time.h>
#include <volk/volk.h>

#include "dsp.h"

#define NSEC_PER_MSEC 1000000L

spectrum_t* spec_create(size_t len) {
    int ret;
    spectrum_t *spec;
    
    spec = malloc(sizeof(spectrum_t));
    assert(spec != NULL);

    ret = pthread_mutex_init(&(spec->mutex), NULL);
    assert(ret == 0);

    spec->len = len;
    // PSD output buffer
    spec->psd = volk_malloc(len*sizeof(float), 32);
    assert(spec->psd != NULL);
    // Window buffer
    spec->win = volk_malloc(len*sizeof(float), 32);
    assert(spec->win != NULL);
    
    // Create window
    window(spec->win, (int)len);
    // Prerotate by pi to center FFT output at 0Hz.
    rotate_pi(spec->win, (int)len);
    
    fftwf_alloc_complex(len);
    
    spec->fft_in = fftwf_alloc_complex(len);
    spec->fft_out = fftwf_alloc_complex(len);
    assert(spec->fft_in != NULL && spec->fft_out != NULL);

    spec->rb = rb_create(len*sizeof(float complex));
    assert(spec->rb != NULL);
    
    spec->fft_fwd_plan = fftwf_plan_dft_1d((int)len,
                                           spec->fft_in,
                                           spec->fft_out,
                                           FFTW_FORWARD,
                                           FFTW_MEASURE);

    return spec;
}


static void cleanup_unlock_mutex(void *arg)
{
    pthread_mutex_unlock((pthread_mutex_t*)arg);
    printf("spectrum cleanup\n");
}

void *worker(void *arg) {
    spectrum_t *spec = (spectrum_t*) arg;
    pthread_cleanup_push(cleanup_unlock_mutex, &(spec->mutex));
    
    struct timespec dt;
    dt.tv_sec = 0;
    dt.tv_nsec = 250 * NSEC_PER_MSEC; // 250ms = 4 specs/s

    while(1) {
        nanosleep(&dt, NULL);
        // Take mutex
        pthread_mutex_lock(&spec->mutex);
        // Get a pointer in the ringbuffer len samples behind current writeptr
        float complex *in = rb_histptr(spec->rb, spec->len * sizeof(float complex));
        // Multiply preshifted window
        volk_32fc_32f_multiply_32fc(spec->fft_in, in, spec->win, (int)spec->len);
        pthread_mutex_unlock(&spec->mutex);
        
        // Perform FFT on this thread
        fftwf_execute(spec->fft_fwd_plan);
        // Calculate power spectrum
        volk_32fc_s32f_power_spectrum_32f(spec->psd, spec->fft_out, spec->len, (int)spec->len);
        printf("%f!\n", spec->psd[32768/2]);
    }

    pthread_cleanup_pop(1);
    return NULL;
}

void spec_stop(spectrum_t *spec) {
    pthread_cancel(spec->thread);
    pthread_join(spec->thread, NULL);
}

int spec_start(spectrum_t *spec) {
    return pthread_create(&spec->thread,
                          NULL,
                          &worker,
                          spec);
}

void spec_destroy(spectrum_t *spec) {
    pthread_mutex_destroy(&spec->mutex);
    free(spec);
}

void spec_add_buffer(spectrum_t *spec, float complex *in, size_t len) {
    pthread_mutex_lock(&spec->mutex);
    rb_memcpy(spec->rb, in, len*sizeof(float complex));
    pthread_mutex_unlock(&spec->mutex);
}
