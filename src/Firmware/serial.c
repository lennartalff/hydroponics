#include "serial.h"

#include <avr/io.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <util/crc16.h>

#include "cobs.h"
#include "common.h"
#include "led.h"
#include "packet.h"
#include "uart.h"

#define LOG_FORMAT "[%s][%s] %s"
#define LOG_FORMAT_MAX_LEN 32
#define LOG_MAX_LEN (256 - 64)
#define BAUD 250000
#define SERIAL_RX_BUFFER_SIZE 256

static uint8_t serial_initialized = 0;
static uint8_t log_level = SERIAL_LOG_LVL_DEBUG;

static const char *_get_level_string(serial_log_level_t level) {
    switch (level) {
        case SERIAL_LOG_LVL_DEBUG:
            return "DEBUG";
        case SERIAL_LOG_LVL_INFO:
            return "INFO";
        case SERIAL_LOG_LVL_WARNING:
            return "WARNING";
        case SERIAL_LOG_LVL_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

static const char *_get_src_string(serial_log_source_t source) {
    switch (source) {
        case SERIAL_SRC_EC:
            return "EC";
        case SERIAL_SRC_PH:
            return "PH";
        case SERIAL_SRC_UART:
            return "UART";
        case SERIAL_SRC_SERIAL:
            return "SERIAL";
        case SERIAL_SRC_GENERAL:
            return "GENERAL";
        case SERIAL_SRC_OWI:
            return "OWI";
        case SERIAL_SRC_TWI:
            return "TWI";
        default:
            return "UNKNOWN";
    }
}

static void _log(serial_log_level_t level, serial_log_source_t source,
                 const char *format, va_list args) {
    char new_format[LOG_MAX_LEN];
    char output[LOG_MAX_LEN];
    char encoded_output[LOG_MAX_LEN + 3];
    uint8_t i;
    int length;
    packet_t packet;

    length =
        snprintf(new_format, LOG_MAX_LEN, LOG_FORMAT, _get_level_string(level),
                 _get_src_string(source), format);
    if (length < 0) {
        return;
    }

    i = vsnprintf(output, LOG_MAX_LEN, new_format, args);
    encode_logging(&packet, output);
    serial_send_packet(&packet);
}

void serial_init() {
    if (!serial_initialized) {
        uart_init(0, BAUD);
        serial_initialized = 1;
        serial_info(SERIAL_SRC_SERIAL, "Init complete.");

    } else {
        serial_debug(SERIAL_SRC_SERIAL,
                     "serial_init() called, but already initialized.");
    }
}

void serial_send_raw(uint8_t *data) {
    uart_0_puts((char *)data);
    uart_0_putc('\0');
}

void serial_send_packet(packet_t *packet) {
    uint8_t serialized_data[255];
    uint8_t encoded_data[255];
    uint8_t length;
    length = packet_serialize(packet, serialized_data);
    length = cobs_encode(serialized_data, encoded_data, length);
    serial_send_raw(encoded_data);
}

void serial_debug(serial_log_source_t source, const char *format, ...) {
    va_list args;
    if (log_level > SERIAL_LOG_LVL_DEBUG) {
        return;
    }
    va_start(args, format);
    _log(SERIAL_LOG_LVL_DEBUG, source, format, args);
    va_end(args);
}

void serial_info(serial_log_source_t source, const char *format, ...) {
    va_list args;
    if (log_level > SERIAL_LOG_LVL_INFO) {
        return;
    }
    va_start(args, format);
    _log(SERIAL_LOG_LVL_INFO, source, format, args);
    va_end(args);
}

void serial_warning(serial_log_source_t source, const char *format, ...) {
    va_list args;
    if (log_level > SERIAL_LOG_LVL_WARNING) {
        return;
    }
    va_start(args, format);
    _log(SERIAL_LOG_LVL_WARNING, source, format, args);
    va_end(args);
}

void serial_error(serial_log_source_t source, const char *format, ...) {
    va_list args;
    if (log_level > SERIAL_LOG_LVL_ERROR) {
        return;
    }
    va_start(args, format);
    _log(SERIAL_LOG_LVL_ERROR, source, format, args);
    va_end(args);
}

static void _write_payload(const uint8_t *src, uint8_t *dst, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        dst[i] = src[i];
    }
}

uint8_t serial_read_raw(uint8_t *data) {
    uint8_t i = 0;
    while (1) {
        data[i] = uart_0_getc();
        if (data[i] == 0) {
            serial_debug(SERIAL_SRC_SERIAL, "Received packet frame.");
            i = cobs_decode(data, data);
            return i;
        }
        i++;
        if (i == 0) {
            serial_warning(SERIAL_SRC_SERIAL, "Receive buffer overflow,");
        }
    }
}

void serial_read_packet(packet_t *packet) {
    uint8_t length;
    return_status_t status;
    uint8_t receive_buffer[255];
    while (1) {
        length = serial_read_raw(receive_buffer);
        status = packet_deserialize(packet, receive_buffer, length);
        if (status == RET_SUCCESS) {
            serial_info(SERIAL_SRC_SERIAL, "Received valid packet with ID %hu",
                        packet->id);
            return;
        }
    }
}
