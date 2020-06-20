#include "ec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "common.h"
#include "serial.h"
#include "twi.h"

#define TWI_ADDRESS 0x65
#define MAX_STRING_LENGTH 64

#define RESPONSE_NO_DATA_TO_SEND 255
#define RESPONSE_STILL_PROCESSING 254
#define RESPONSE_SYNTAX_ERROR 2
#define RESPONSE_SUCCESS 1

#define WAIT_TIME_READ_MS 900
#define WAIT_TIME_GENERAL_MS 300

#define ENABLE_PIN PJ2
#define ENABLE_PORT PORTJ

void ec_init() {
    twi_init();
    DDR_REGISTER(ENABLE_PORT) |= (1 << ENABLE_PIN);
    ENABLE_PORT &= ~(1 << ENABLE_PIN);
    _delay_ms(100);
    ENABLE_PORT |= (1 << ENABLE_PIN);
    _delay_ms(1000);
}

return_status_t ec_send_command(char *command) {
    serial_debug(SERIAL_SRC_EC, "Writing command: %s", command);
    return_status_t status;
    status = twi_start_write(TWI_ADDRESS);
    ASSERT_SUCCESS_TWI_STOP(status);
    uint8_t i = 0;
    while (command[i] != '\0') {
        status = twi_write_byte((uint8_t)command[i++]);
        ASSERT_SUCCESS_TWI_STOP(status);
    }
    twi_stop();
    return RET_SUCCESS;
}

return_status_t ec_read_raw(char *response_string) {
    return_status_t status;
    status = twi_start_read(TWI_ADDRESS);
    ASSERT_SUCCESS_TWI_STOP(status);
    uint8_t byte;
    uint8_t i = 0;
    do {
        status = twi_read_ack(&byte);
        ASSERT_SUCCESS_TWI_STOP(stauts);
        serial_debug(SERIAL_SRC_EC, "Read byte: %d", byte);
        response_string[i++] = (char)byte;
    } while (byte != 0);
    twi_read_nack(&byte);
    twi_stop();
    return RET_SUCCESS;
}

return_status_t ec_parse_response(char *response_string, uint8_t *code,
                                  char *data_string) {
    uint8_t i = 0;
    *code = (uint8_t)response_string[0];
    do {
        data_string[i] = response_string[i + 1];
        i++;
    } while (response_string[i] != '\0');
    return RET_SUCCESS;
}

return_status_t ec_read_response(char *response_buffer, uint8_t *code,
                                 char *data_string) {
    return_status_t status;
    serial_debug(SERIAL_SRC_EC, "Start reading raw data");
    status = ec_read_raw(response_buffer);
    ASSERT_SUCCESS(status);
    serial_debug(SERIAL_SRC_EC, "Start parsing raw data");
    status = ec_parse_response(response_buffer, code, data_string);
    ASSERT_SUCCESS(status);
    return RET_SUCCESS;
}

return_status_t ec_read_ec(uint32_t *value) {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    status = ec_send_command("R");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_READ_MS);
    while (1) {
        status = ec_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        serial_debug(SERIAL_SRC_EC, "Response code: %d", code);
        if (code == RESPONSE_SUCCESS) {
            *value = (uint32_t)(atof(buffer));
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_EC_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_EC_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_EC,
                         "Reading of EC takes longer than expected...");
            continue;
        }
    }
}

return_status_t ec_calibration_dry() {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    status = ec_send_command("Cal,dry");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_READ_MS);
    while (1) {
        status = ec_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_EC, "Dry calibration done.");
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_EC_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_EC_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_EC,
                         "Performing dry calibration takes longer than "
                         "expected...");
            continue;
        }
    }
}

return_status_t ec_calibration_low() {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    status = ec_send_command("Cal,low,12880");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_READ_MS);
    while (1) {
        status = ec_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_EC, "Lowpoint calibration done.");
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_EC_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_EC_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_EC,
                         "Performing lowpoint calibration takes longer than "
                         "expected...");
            continue;
        }
    }
}

return_status_t ec_calibration_high() {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    status = ec_send_command("Cal,high,80000");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_READ_MS);
    while (1) {
        status = ec_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_EC, "Highpoint calibration done.");
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_EC_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_EC_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_EC,
                         "Performing highpoint calibration takes longer than "
                         "expected...");
            continue;
        }
    }
}

return_status_t ec_calibration_clear() {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    status = ec_send_command("Cal,clear");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_READ_MS);
    while (1) {
        status = ec_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_EC, "Calibration cleared.");
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_EC_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_EC_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_EC,
                         "Clearing calibration takes longer than "
                         "expected...");
            continue;
        }
    }
}

return_status_t ec_calibration_format(uint8_t *n_strings, uint8_t *n_bytes) {
    char buffer[MAX_STRING_LENGTH];
    char *tok;
    uint8_t code;
    return_status_t status;
    status = ec_send_command("Export,?");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_GENERAL_MS);
    while (1) {
        status = ec_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_EC, "Got calibration format.");
            tok = strtok(buffer, ",");
            *n_strings = (uint8_t)atoi(tok);
            tok = strtok(NULL, ",");
            *n_bytes = (uint8_t)atoi(tok);
            serial_info(SERIAL_SRC_EC,
                        "Calibration format: %d bytes in %d strings.", *n_bytes,
                        *n_strings);
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_EC_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_EC_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(
                SERIAL_SRC_EC,
                "Getting calibration format takes longer than expected...");
            continue;
        }
    }
}

return_status_t ec_calibration_export(uint8_t *calib_data) {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    uint8_t n_bytes;
    uint8_t data_index = 0;
    return_status_t status;

    while (1) {
        status = ec_send_command("Export");
        ASSERT_SUCCESS(status);
        status = ec_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            if (strcmp(buffer, "*DONE") == 0) {
                calib_data[data_index] = 0;
                serial_info(SERIAL_SRC_EC,
                            "Got calibration string of length: %d",
                            data_index + 1);
                return RET_SUCCESS;
            }
            n_bytes = strlen(buffer);
            for (uint8_t i = 0; i < n_bytes; i++) {
                calib_data[data_index++] = (uint8_t)buffer[i];
            }
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_EC_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_EC_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(
                SERIAL_SRC_EC,
                "Getting calibration string takes longer than expected...");
            continue;
        }
    }
}

return_status_t ec_temperature_compensation(float temperature) {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    sprintf(buffer, "T,%.2f", temperature);
    status = ec_send_command(buffer);
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_READ_MS);
    while (1) {
        status = ec_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_EC, "Temperature compensation set to %.2f",
                        temperature);
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_EC_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_EC_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_EC,
                         "Setting temperature compensation takes longer than "
                         "expected...");
            continue;
        }
    }
}