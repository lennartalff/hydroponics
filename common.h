#ifndef COMMON_H_
#define COMMON_H_

#include <avr/io.h>

#define PIN_REGISTER(x) (*(&(x) - 2))
#define DDR_REGISTER(x) (*(&(x) - 1))

#define LOWER_BYTE(x) ((uint8_t) ((x) & 0xFF))
#define UPPER_BYTE(x) ((uint8_t) ((x) >> 8))

#endif
