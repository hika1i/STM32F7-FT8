//
// Created by PYL on 2019/7/21.
//

#include "wave.h"

// Transform signal in floating point format (-1 to +1) 
// to 16-bit signed integers as PCM data flow.
void wave2pcm(const float *signal, int num_samples, int sample_rate, int16_t *pcmdata)
{
    for (int i = 0; i < num_samples; i++)
    {
        float x = signal[i];
        if (x > 1.0) x = 1.0;
        else if (x < -1.0) x = -1.0;
        pcmdata[i] = (int) (0.5 + (x * 32767.0));
    }
}
