project('spieserver', 'c')

# cc = meson.get_compiler('c')
c_flags = ['-O3', '-ffast-math']
add_project_arguments(c_flags, language: 'c')

volk_dep = dependency('volk', required: true)
fftwf_dep = dependency('fftw3f', required: true)
airspyhf_dep = dependency('libairspyhf', required: true)

sources = ['main.c', 'dsp.c', 'fir.c', 'agc.c',
    'doublemap.c', 'ringbuffer.c', 'spectrum.c', 'rtp.c', 'fft_filter.c', 'fft_xlating.c']

executable('spieserver', sources,
    dependencies: [volk_dep, fftwf_dep, airspyhf_dep,])
