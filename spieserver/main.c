//
//  main.c
//  spieserver
//
//  Created by Albin Stigö on 2019-12-07.
//  Copyright © 2019 Albin Stigo. All rights reserved.
//

#include <stdio.h>
#include <airspyhf.h>
#include <unistd.h>
#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "fir.h"
#include "spectrum.h"
#include "rtp.h"
#include "fft_filter.h"
#include "fft_xlating.h"

typedef struct {
    int sd;
    fft_xlating_t *xlating;
    fft_filter_t *ff;
    fir_t *fir;
    spectrum_t *spec;
} server_context_t;

int airspy_cb(airspyhf_transfer_t* tr) {
    const int N = 2048/32;
    float complex out[N];
    
    server_context_t *ctx = (server_context_t*) tr->ctx;
    
    // Send buffer to spectrum thread
    spec_add_buffer(ctx->spec, (float complex*)tr->samples, tr->sample_count);
    // Channel filter
    fft_xlate_execute(ctx->xlating, (float complex*)tr->samples, out);
    
    // Demodulation filter
    fft_filt_execute(ctx->ff, out, out);
    
    ssize_t ret = send(ctx->sd, out, N*sizeof(float complex), MSG_DONTWAIT); // last is flags

    if(tr->dropped_samples) {
        printf("%llu\n", tr->dropped_samples);
    }
    
    return 0;
}

int main(int argc, const char * argv[]) {
    int ret;
    
    int sd = create_socket();
    
    airspyhf_device_t *dev;
    
    // Channel filter
    fft_xlating_t xlating;
    fft_xlate_init(&xlating, 801, 2048, 32);
    fft_xlate_set_proto(&xlating, 9000/768000.);
    fft_xlate_set_center(&xlating, 74800/768000.);
    
    fft_filter_t ff;
    fft_filt_init(&ff, 501, 64, 1, 0);
    fft_filt_set_proto(&ff, 1400/24000.);
    fft_filt_set_center(&ff, 800/24000.);

    // 2nd filter
    fir_t fir;
    // 2nd time domain filter
    fir_new(&fir, 501, 64, 1);

    // Spectrum
    spectrum_t *spec;
    spec = spec_create(1 << 15);
    ret = spec_start(spec);
    assert(ret == 0);
    
    ret = airspyhf_open(&dev);
    assert(airspyhf_get_output_size(dev) == 2048);

    airspyhf_set_freq(dev, 7000000);
    airspyhf_set_samplerate(dev, 768000);

    server_context_t ctx;
    ctx.xlating = &xlating;
    ctx.ff = &ff;
    ctx.fir = &fir;
    ctx.spec = spec;
    ctx.sd = sd;
    
    airspyhf_start(dev, &airspy_cb, &ctx);
    
    getchar();

    airspyhf_close(dev);
    
    spec_stop(spec);
    spec_destroy(spec);
    fft_xlate_free(&xlating);
    fft_filt_free(&ff);
    fir_free(&fir);

    return 0;
}
