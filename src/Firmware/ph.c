#include "ph.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "serial.h"
#include "twi.h"

#define TWI_ADDRESS 0x64
#define MAX_STRING_LENGTH 64

#define RESPONSE_NO_DATA_TO_SEND 255
#define RESPONSE_STILL_PROCESSING 254
#define RESPONSE_SYNTAX_ERROR 2
#define RESPONSE_SUCCESS 1

#define WAIT_TIME_READ_MS 900
#define WAIT_TIME_GENERAL_MS 300

#define ENABLE_PORT PORTH
#define ENABLE_PIN PH2

void ph_init() {
    twi_init();
    DDR_REGISTER(ENABLE_PORT) |= (1 << ENABLE_PIN);
    ENABLE_PORT &= ~(1 << ENABLE_PIN);
    _delay_ms(100);
    ENABLE_PORT |= (1 << ENABLE_PIN);
    _delay_ms(1000);
}

return_status_t ph_send_command(char *command) {
    serial_debug(SERIAL_SRC_PH, "Writing command: %s", command);
    return_status_t status;
    status = twi_start_write(TWI_ADDRESS);
    ASSERT_SUCCESS_TWI_STOP(status);
    uint8_t i = 0;
    while (command[i] != '\0') {
        status = twi_write_byte((uint8_t)command[i++]);
        serial_debug(SERIAL_SRC_PH, "Byte writte: %d", command[i - 1]);
        ASSERT_SUCCESS_TWI_STOP(status);
    }
    twi_stop();
    return RET_SUCCESS;
}

return_status_t ph_read_raw(char *response_string) {
    return_status_t status;
    status = twi_start_read(TWI_ADDRESS);
    ASSERT_SUCCESS_TWI_STOP(status);
    uint8_t byte;
    uint8_t i = 0;
    do {
        status = twi_read_ack(&byte);
        ASSERT_SUCCESS_TWI_STOP(status);
        serial_debug(SERIAL_SRC_PH, "Read byte: %d", byte);
        response_string[i++] = (char)byte;
    } while (byte != 0);
    twi_read_nack(&byte);
    twi_stop();
    return RET_SUCCESS;
}

return_status_t ph_parse_reponse(char *response_string, uint8_t *code,
                                 char *data_string) {
    uint8_t i = 0;
    *code = (uint8_t)response_string[0];
    do {
        data_string[i] = response_string[i + 1];
        i++;
    } while (response_string[i] != '\0');
    return RET_SUCCESS;
}

/**
 * @brief Reads the reponse from the sensor.
 *
 * @p reponse_buffer and @p data_string can be the same. Then the read operation
 * will be inplace.
 *
 * @param response_buffer
 * @param code
 * @param data_string
 * @return return_status_t
 */
return_status_t ph_read_response(char *response_buffer, uint8_t *code,
                                 char *data_string) {
    return_status_t status;
    serial_debug(SERIAL_SRC_PH, "Start reading raw data");
    status = ph_read_raw(response_buffer);
    ASSERT_SUCCESS(status);
    serial_debug(SERIAL_SRC_PH, "Start parsing raw data");
    status = ph_parse_reponse(response_buffer, code, data_string);
    ASSERT_SUCCESS(status);
    return RET_SUCCESS;
}

return_status_t ph_read_ph(uint32_t *value) {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    status = ph_send_command("R");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_READ_MS);
    while (1) {
        status = ph_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        serial_debug(SERIAL_SRC_PH, "Response code: %d", code);
        if (code == RESPONSE_SUCCESS) {
            *value = (uint32_t)(atof(buffer) * 1000);
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_PH_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_PH_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_PH,
                         "Reading of PH takes longer than expected...");
            continue;
        }
    }
}

return_status_t ph_calibration_mid() {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    status = ph_send_command("Cal,mid,7.00");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_READ_MS);
    while (1) {
        status = ph_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_PH, "Midpoint calibration done.");
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_PH_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_PH_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_PH,
                         "Performing midpoint calibration takes longer than "
                         "expected...");
            continue;
        }
    }
}

return_status_t ph_calibration_low() {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    status = ph_send_command("Cal,low,4.00");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_READ_MS);
    while (1) {
        status = ph_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_PH, "Lowpoint calibration done.");
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_PH_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_PH_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_PH,
                         "Performing lowpoint calibration takes longer than "
                         "expected...");
            continue;
        }
    }
}

return_status_t ph_calibration_high() {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    status = ph_send_command("Cal,high,10.00");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_READ_MS);
    while (1) {
        status = ph_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_PH, "Highpoint calibration done.");
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_PH_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_PH_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_PH,
                         "Performing highpoint calibration takes longer than "
                         "expected...");
            continue;
        }
    }
}

return_status_t ph_calibration_clear() {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    status = ph_send_command("Cal,clear");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_GENERAL_MS);
    while (1) {
        status = ph_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_PH, "Calibration cleared.");
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_PH_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_PH_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_PH,
                         "Clearing calibration takes longer than expected...");
            continue;
        }
    }
}

return_status_t ph_calibration_format(uint8_t *n_strings, uint8_t *n_bytes) {
    char buffer[MAX_STRING_LENGTH];
    char *tok;
    uint8_t code;
    return_status_t status;
    status = ph_send_command("Export,?");
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_GENERAL_MS);
    while (1) {
        status = ph_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_PH, "Got calibration format.");
            tok = strtok(buffer, ",");
            *n_strings = (uint8_t)atoi(tok);
            tok = strtok(NULL, ",");
            *n_bytes = (uint8_t)atoi(tok);
            serial_info(SERIAL_SRC_PH,
                        "Calibration format: %d bytes in %d strings.", *n_bytes,
                        *n_strings);
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_PH_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_PH_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(
                SERIAL_SRC_PH,
                "Getting calibration format takes longer than expected...");
            continue;
        }
    }
}

return_status_t ph_calibration_export(uint8_t *calib_data) {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    uint8_t n_bytes;
    uint8_t data_index = 0;
    return_status_t status;

    while (1) {
        status = ph_send_command("Export");
        ASSERT_SUCCESS(status);
        status = ph_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            if (strcmp(buffer, "*DONE") == 0) {
                calib_data[data_index] = 0;
                serial_info(SERIAL_SRC_PH,
                            "Got calibration string of length: %d",
                            data_index + 1);
                return RET_SUCCESS;
            }
            n_bytes = strlen(buffer);
            for (uint8_t i = 0; i < n_bytes; i++) {
                calib_data[data_index++] = (uint8_t)buffer[i];
            }
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_PH_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_PH_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(
                SERIAL_SRC_PH,
                "Getting calibration string takes longer than expected...");
            continue;
        }
    }
}

return_status_t ph_temperature_compensation(float temperature) {
    char buffer[MAX_STRING_LENGTH];
    uint8_t code;
    return_status_t status;
    sprintf(buffer, "T,%.2f", temperature);
    status = ph_send_command(buffer);
    ASSERT_SUCCESS(status);
    _delay_ms(WAIT_TIME_READ_MS);
    while (1) {
        status = ph_read_response(buffer, &code, buffer);
        ASSERT_SUCCESS(status);
        if (code == RESPONSE_SUCCESS) {
            serial_info(SERIAL_SRC_PH, "Temperature compensation set to %.2f", temperature);
            return RET_SUCCESS;
        } else if (code == RESPONSE_NO_DATA_TO_SEND) {
            return RET_PH_NO_RESPONSE;
        } else if (code == RESPONSE_SYNTAX_ERROR) {
            return RET_PH_SYNTAX_ERR;
        } else if (code == RESPONSE_STILL_PROCESSING) {
            serial_debug(SERIAL_SRC_PH,
                         "Setting temperature compensation takes longer than "
                         "expected...");
            continue;
        }
    }
}