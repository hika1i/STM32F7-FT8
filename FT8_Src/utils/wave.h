#ifndef __WAVE_H
#define __WAVE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// Transform signal in floating point format (-1 to +1) 
// to 16-bit signed integers as PCM data flow.
void wave2pcm(const float *signal, int num_samples, int sample_rate, int16_t *pcmdata);

// Load signal in floating point format (-1 .. +1) as a WAVE file using 16-bit signed integers.
int pcm2wave(float *signal, int *num_samples, int *sample_rate, const int16_t *pcmdata);

#ifdef __cplusplus
}
#endif

#endif
