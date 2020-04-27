//
// Created by PYL on 2019/7/21.
//
#ifndef __TEXT_H
#define __TEXT_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

char to_upper(char c);

uint8_t is_digit(char c);

uint8_t is_letter(char c);

uint8_t is_space(char c);

uint8_t in_range(char c, char min, char max);

uint8_t starts_with(const char *string, const char *prefix);

uint8_t equals(const char *string1, const char *string2);

int char_index(const char *string, char c);

// Text message formatting:
//   - replaces lowercase letters with uppercase
//   - merges consecutive spaces into single space
void fmtmsg(char *msg_out, const char *msg_in);

// Parse a 2 digit integer from string
int dd_to_int(const char *str, int length);

// Convert a 2 digit integer to string
void int_to_dd(char *str, int value, int width, uint8_t full_sign);

#ifdef __cplusplus
}
#endif

#endif
