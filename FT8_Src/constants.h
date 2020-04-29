//
// Created by PYL on 2019/7/19.
//

#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

//LDPC sizes
#define LDPC_N 174
#define LDPC_K 91
#define LDPC_M 83 //LDPC_M = LDPC_N - LDPC_K
#define K_BYTES 12 // (LDPC_K + 7)/8
//
//FT8 symbol counts
#define ND 58
#define NS 21
#define NN 79 //NN = NS + ND
#define CRC_POLYNOMIAL 0x2757
#define CRC_WIDTH 14

//const uint8_t LDPC_N = 174;
//const uint8_t LDPC_K = 91;
//const uint8_t LDPC_M = 83;
//const uint8_t K_BYTES = 12;
//const uint8_t ND = 58;
//const uint8_t NS = 22;
//const uint8_t NN = 79;

// Define CRC parameters
//const uint16_t CRC_POLYNOMIAL = 0x2757;  // CRC-14 polynomial without the leading (MSB) 1
//const int CRC_WIDTH = 14;

//LDPC(174, 91) code

//Gray map
extern const uint8_t graymap[8];

//Costas 7x7 tone pattern
extern const uint8_t icos7[7];

//LDPC 174 parity arrays
extern const uint8_t Mn[LDPC_N][3];

extern const uint8_t Nm[LDPC_M][7];

extern const uint8_t nrw[LDPC_M];

// Parity generator matrix for (174,91) LDPC code, stored in bitpacked format (MSB first)
extern const uint8_t kGenerator[LDPC_M][K_BYTES];
extern const int16_t raw_data[];
extern const float ffttest[];

#ifdef __cplusplus
}
#endif

#endif //FT8_C_VERSION_CONSTANTS_H
