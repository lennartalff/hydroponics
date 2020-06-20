#ifndef COBS_H_
#define COBS_H_

#include <stdint.h>

uint8_t cobs_encode(uint8_t *src, uint8_t *dst, uint8_t length);

uint8_t cobs_decode(uint8_t *src, uint8_t *dst);

#endif