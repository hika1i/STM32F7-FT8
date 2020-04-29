#ifndef __UNPACK_H
#define __UNPACK_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

// message should have at least 19 bytes allocated (18 characters + zero terminator)
int unpack77(const uint8_t *a77, char *message);

#ifdef __cplusplus
}
#endif

#endif
