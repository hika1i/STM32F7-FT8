#ifndef __PACK_H
#define __PACK_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

// Pack FT8 text message into 72 bits
// [IN] msg      - FT8 message (e.g. "CQ TE5T KN01")
// [OUT] c77     - 10 byte array to store the 77 bit payload (MSB first)
int pack77(const char *msg, uint8_t *c77);

#ifdef __cplusplus
}
#endif

#endif