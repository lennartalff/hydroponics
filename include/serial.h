#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#include "owi.h"
#include "packet.h"

#define MAX_PACKET_LENGTH 256

typedef enum {
    SERIAL_LOG_LVL_DEBUG = 1,
    SERIAL_LOG_LVL_INFO = 2,
    SERIAL_LOG_LVL_WARNING = 4,
    SERIAL_LOG_LVL_ERROR = 8
} serial_log_level_t;


typedef enum {
    SERIAL_SRC_PH,
    SERIAL_SRC_EC,
    SERIAL_SRC_UART,
    SERIAL_SRC_SERIAL,
    SERIAL_SRC_OWI,
    SERIAL_SRC_GENERAL,
    SERIAL_SRC_TWI
} serial_log_source_t;


void serial_init();
void serial_debug(serial_log_source_t source, const char *format, ...);
void serial_info(serial_log_source_t source, const char *format, ...);
void serial_warning(serial_log_source_t source, const char *format, ...);
void serial_error(serial_log_source_t source, const char *format, ...);
void serial_send_raw(uint8_t *data);
void serial_send_packet(packet_t *packet);
void serial_read_packet(packet_t *packet);
void serial_send_packet(packet_t *packet);

#endif