//
// Created by PYL on 2019/7/21.
//

#ifndef __FILTER_H
#define __FILTER_H

#ifdef __cplusplus
 extern "C" {
#endif

float hann_i(int i, int n);

float hamming_i(int i, int n);

float blackman_i(int i, int n);

void normalize_signal(float *signal, int num_samples);

#ifdef __cplusplus
}
#endif

#endif
