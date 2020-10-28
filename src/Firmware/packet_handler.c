#include "packet_handler.h"

#include <stdlib.h>

#include "ec.h"
#include "owi.h"
#include "packet.h"
#include "ph.h"
#include "pwm.h"
#include "relays.h"
#include "serial.h"

void handle_cmd_owi_set_res(packet_t *packet) {
    return_status_t status;
    uint8_t resolution;
    decode_cmd_owi_set_res(packet, &resolution);
    status = owi_set_resolution_all(resolution);
    if (status != RET_SUCCESS) {
        switch (status) {
            case RET_OWI_UNKNOWN_RES:
                serial_warning(
                    SERIAL_SRC_OWI,
                    "Unknown resolution '%d'. Resolution was not set.",
                    packet->payload[0]);
                break;
            case RET_OWI_NO_PRESENCE:
                serial_warning(SERIAL_SRC_OWI,
                               "No presence pulse detected. Resolution was not "
                               "set.");
                break;

            default:
                serial_warning(SERIAL_SRC_OWI, "Unexpected exit code: %d",
                               status);
                break;
        }
        return;
    }
}

void handle_cmd_owi_get_res(packet_t *packet) {
    owi_resolution_t resolution;
    owi_get_resolution_all(&resolution);

    encode_response_owi_get_res(packet, resolution);

    serial_send_packet(packet);
}

void handle_cmd_owi_measure(packet_t *packet) {
    serial_info(SERIAL_SRC_OWI, "Handling measure");
    uint8_t count = 1;
    return_status_t status;
    ds18b20_t device;
    owi_start_conversion();
    owi_wait_conversion(NULL);
    status = owi_search_first();
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_OWI,
                     "Could not measure temperature. Exit code: %d", status);
        return;
    }
    owi_get_buffered_rom(device.rom);
    owi_get_resolution_all(&device.resolution);
    status = owi_read_temperature(&device);
    encode_data_owi(packet, device.rom, device.temperature);
    serial_send_packet(packet);
    while (status == RET_SUCCESS) {
        status = owi_search_next();
        if (status != RET_SUCCESS) {
            // break loop. We have read out all temperatures
            if (status != RET_OWI_SEARCH_LAST_DEVICE) {
                serial_error(SERIAL_SRC_OWI,
                             "Could not measure temeprature. Exit code: %d",
                             status);
            }
            serial_debug(SERIAL_SRC_OWI, "Read temperature from %d devices",
                         count);
            return;
        }
        count++;
        owi_get_buffered_rom(device.rom);
        status = owi_read_temperature(&device);
        encode_data_owi(packet, device.rom, device.temperature);
        serial_send_packet(packet);
    }
    serial_error(SERIAL_SRC_OWI, "Could not measure temperature. Exit code: %d",
                 status);
}

void handle_cmd_ec_measure(packet_t *packet) {
    uint32_t ec;
    return_status_t status;
    status = ec_read_ec(&ec);
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_EC, "Could not measre EC. Exit code: %d",
                     status);
        return;
    }
    encode_data_ec(packet, ec);
    serial_send_packet(packet);
}

void handle_cmd_ec_import_calib(packet_t *packet) {}
void handle_cmd_ec_export_calib(packet_t *packet) {}
void handle_cmd_ec_clear_calib(packet_t *packet) {
    return_status_t status;
    status = ec_calibration_clear();
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_EC,
                     "Could not clear calibration. Exit code: %d", status);
    }
}
void handle_cmd_ec_calib_dry(packet_t *packet) {
    return_status_t status;
    status = ec_calibration_dry();
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_EC, "Could not calibrate dry. Exit code: %d",
                     status);
    }
}

void handle_cmd_ec_calib_low(packet_t *packet) {
    return_status_t status;
    status = ec_calibration_low();
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_EC, "Could not calibrate low. Exit code: %d",
                     status);
    }
}

void handle_cmd_ec_calib_high(packet_t *packet) {
    return_status_t status;
    status = ec_calibration_high();
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_EC, "Could not calibrate high. Exit code: %d",
                     status);
    }
}

void handle_cmd_ec_compensation(packet_t *packet) {
    return_status_t status;
    float t;
    decode_cmd_ec_compensation(packet, &t);
    status = ec_temperature_compensation(t);
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_EC,
                     "Could not set temperature compensation. Exit code. %d",
                     status);
    }
}

void handle_cmd_ph_measure(packet_t *packet) {
    uint32_t ph;
    return_status_t status;
    status = ph_read_ph(&ph);
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_PH, "Could not measure PH. Exit code: %d",
                     status);
        return;
    }
    encode_data_ph(packet, ph);
    serial_send_packet(packet);
}
void handle_cmd_ph_import_calib(packet_t *packet) {}
void handle_cmd_ph_export_calib(packet_t *packet) {}
void handle_cmd_ph_clear_calib(packet_t *packet) {
    return_status_t status;
    status = ph_calibration_clear();
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_PH,
                     "Could not clear calibration. Exit code: %d", status);
    }
}
void handle_cmd_ph_calib_low(packet_t *packet) {
    return_status_t status;
    status = ph_calibration_low();
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_PH, "Could not calibrate low. Exit code: %d",
                     status);
    }
}

void handle_cmd_ph_calib_mid(packet_t *packet) {
    return_status_t status;
    status = ph_calibration_mid();
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_PH, "Could not calibrate mid. Exit code: %d",
                     status);
    }
}

void handle_cmd_ph_calib_high(packet_t *packet) {
    return_status_t status;
    status = ph_calibration_high();
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_PH, "Could not calibrate high. Exit code: %d",
                     status);
    }
}

void handle_cmd_ph_compensation(packet_t *packet) {
    return_status_t status;
    float t;
    decode_cmd_ph_compensation(packet, &t);
    status = ph_temperature_compensation(t);
    if (status != RET_SUCCESS) {
        serial_error(SERIAL_SRC_PH,
                     "Could not set temperature compensation. Exit code. %d",
                     status);
    }
}

void handle_cmd_light_set(packet_t *packet) {
    uint8_t state;
    decode_cmd_light_set(packet, &state);
    relays_set(RELAYS_BLUE, state);
    relays_set(RELAYS_RED, state);
    relays_set(RELAYS_WHITE, state);
}
void handle_cmd_light_get(packet_t *packet) {}
void handle_cmd_light_blue_set(packet_t *packet) {
    uint8_t state;
    decode_cmd_light_blue_set(packet, &state);
    relays_set(RELAYS_BLUE, state);
}
void handle_cmd_light_blue_get(packet_t *packet) {}
void handle_cmd_light_red_set(packet_t *packet) {
    uint8_t state;
    decode_cmd_light_red_set(packet, &state);
    relays_set(RELAYS_RED, state);
}
void handle_cmd_light_red_get(packet_t *packet) {}
void handle_cmd_light_white_set(packet_t *packet) {
    uint8_t state;
    decode_cmd_light_white_set(packet, &state);
    relays_set(RELAYS_WHITE, state);
}
void handle_cmd_light_white_get(packet_t *packet) {}
void handle_cmd_fan_set_speed(packet_t *packet) {
    uint8_t index;
    uint16_t speed;
    decode_cmd_fan_set_speed(packet, &index, &speed);
    serial_info(SERIAL_SRC_GENERAL, "Set fan %d to %d", index, speed);
    pwm_set(index, speed);
}
void handle_cmd_fan_get_speed(packet_t *packet) {}
void handle_cmd_unknown(packet_t *packet) {
    serial_warning(SERIAL_SRC_SERIAL, "Received unknown packet ID");
}
