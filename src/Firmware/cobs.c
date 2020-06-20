#include <stdint.h>
#include <stdlib.h>

uint8_t cobs_encode(uint8_t *src, uint8_t *dst, uint8_t length) {
    const uint8_t *src_start = src;
    uint8_t zero_offset = 1;
    uint8_t src_index = 0, dst_index = 1;
    uint8_t src_byte;

    if (src_start == NULL || dst == NULL) {
        return EXIT_FAILURE;
    }

    while (1) {
        src_byte = src_start[src_index++];
        if (src_byte == 0) {
            dst[dst_index - zero_offset] = zero_offset;
            zero_offset = 1;
        } else {
            dst[dst_index] = src_byte;
            zero_offset++;
        }

        dst_index++;

        // reached end of input data
        if (src_index >= length) {
            dst[dst_index-zero_offset] = zero_offset;
            // add delimiter
            dst[dst_index] = 0;
            return dst_index+1;
        }
    }
}

/**
 * @brief Decodes an array encoded by COBS algorithm
 * 
 * @param src 
 * @param dst 
 * @return uint8_t 
 */
uint8_t cobs_decode(uint8_t *src, uint8_t *dst) {
    const uint8_t *src_start = src;
    uint8_t src_index = 1, dst_index = 0;
    uint8_t zero_offset = src_start[0]-1;

    while (1) {
        if (zero_offset == 0) {
            dst[dst_index] = 0;
            zero_offset = src_start[src_index];
        } else {
            dst[dst_index] = src_start[src_index];
        }
        dst_index++;
        src_index++;
        zero_offset--;
        if (src[src_index] == 0) {
            return dst_index;
        }
    }
}