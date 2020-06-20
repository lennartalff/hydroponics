#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

#include "cobs.h"
#include "ec.h"
#include "led.h"
#include "packet.h"
#include "packet_handler.h"
#include "ph.h"
#include "pwm.h"
#include "relays.h"
#include "serial.h"
#include "twi.h"

#define LED_PORT PORTK

void init_modules();

int main() {
    sei();

    init_modules();
    serial_info(SERIAL_SRC_GENERAL, "All modules initialized");

    packet_t packet;

    while (1) {
        encode_response_ready_request(&packet);
        serial_send_packet(&packet);
        serial_read_packet(&packet);
        serial_debug(SERIAL_SRC_GENERAL, "Handling packet with ID: %hu",
                     packet.id);
        switch (packet.id) {
            case PACKET_ID_CMD_OWI_SET_RES:
                handle_cmd_owi_set_res(&packet);
                break;
            case PACKET_ID_CMD_OWI_GET_RES:
                handle_cmd_owi_get_res(&packet);
                break;
            case PACKET_ID_CMD_OWI_MEASURE:
                handle_cmd_owi_measure(&packet);
                break;
            case PACKET_ID_CMD_EC_MEASURE:
                handle_cmd_ec_measure(&packet);
                break;
            case PACKET_ID_CMD_EC_IMPORT_CALIB:
                handle_cmd_ec_import_calib(&packet);
                break;
            case PACKET_ID_CMD_EC_EXPORT_CALIB:
                handle_cmd_ec_export_calib(&packet);
                break;
            case PACKET_ID_CMD_EC_CLEAR_CALIB:
                handle_cmd_ec_clear_calib(&packet);
                break;
            case PACKET_ID_CMD_EC_CALIB_DRY:
                handle_cmd_ec_calib_dry(&packet);
                break;
            case PACKET_ID_CMD_EC_CALIB_LOW:
                handle_cmd_ec_calib_low(&packet);
                break;
            case PACKET_ID_CMD_EC_CALIB_HIGH:
                handle_cmd_ec_calib_high(&packet);
                break;
            case PACKET_ID_CMD_EC_COMPENSATION:
                handle_cmd_ec_compensation(&packet);
                break;
            case PACKET_ID_CMD_PH_MEASURE:
                handle_cmd_ph_measure(&packet);
                break;
            case PACKET_ID_CMD_PH_IMPORT_CALIB:
                handle_cmd_ph_import_calib(&packet);
                break;
            case PACKET_ID_CMD_PH_EXPORT_CALIB:
                handle_cmd_ph_export_calib(&packet);
                break;
            case PACKET_ID_CMD_PH_CLEAR_CALIB:
                handle_cmd_ph_clear_calib(&packet);
                break;
            case PACKET_ID_CMD_PH_CALIB_LOW:
                handle_cmd_ph_calib_low(&packet);
                break;
            case PACKET_ID_CMD_PH_CALIB_MID:
                handle_cmd_ph_calib_mid(&packet);
                break;
            case PACKET_ID_CMD_PH_CALIB_HIGH:
                handle_cmd_ph_calib_high(&packet);
                break;
            case PACKET_ID_CMD_PH_COMPENSATION:
                handle_cmd_ph_compensation(&packet);
                break;
            case PACKET_ID_CMD_LIGHT_SET:
                handle_cmd_light_set(&packet);
                break;
            case PACKET_ID_CMD_LIGHT_GET:
                handle_cmd_light_get(&packet);
            case PACKET_ID_CMD_LIGHT_BLUE_SET:
                handle_cmd_light_blue_set(&packet);
                break;
            case PACKET_ID_CMD_LIGHT_BLUE_GET:
                handle_cmd_light_blue_get(&packet);
            case PACKET_ID_CMD_LIGHT_RED_SET:
                handle_cmd_light_red_set(&packet);
                break;
            case PACKET_ID_CMD_LIGHT_RED_GET:
                handle_cmd_light_red_get(&packet);
                break;
            case PACKET_ID_CMD_LIGHT_WHITE_SET:
                handle_cmd_light_white_set(&packet);
                break;
            case PACKET_ID_CMD_LIGHT_WHITE_GET:
                handle_cmd_light_white_get(&packet);
                break;
            case PACKET_ID_CMD_FAN_SET_SPEED:
                handle_cmd_fan_set_speed(&packet);
                break;
            case PACKET_ID_CMD_FAN_GET_SPEED:
                handle_cmd_fan_get_speed(&packet);
                break;
            default:
                handle_cmd_unknown(&packet);
                break;
        }
    }
}

void init_modules() {
    serial_init();
    serial_info(SERIAL_SRC_GENERAL, "Booting");

    twi_init();
    return_status_t status;
    for (uint8_t address = 0; address < 120; address++) {
        status = twi_start_write(address);
        if (status == RET_SUCCESS) {
            serial_info(SERIAL_SRC_GENERAL, "Found I2C-device: 0x%02x",
                        address);
        }
        twi_stop();
    }

    serial_info(SERIAL_SRC_GENERAL, "Init led module...");
    led_init();
    led_boot_sequence();

    serial_info(SERIAL_SRC_GENERAL, "Init ec module...");
    ec_init();

    serial_info(SERIAL_SRC_GENERAL, "Init owi module...");
    owi_init(&PORTD, PD4);

    serial_info(SERIAL_SRC_GENERAL, "Init relays module...");
    relays_init();

    serial_info(SERIAL_SRC_GENERAL, "Init pwm module...");
    pwm_init();

    serial_info(SERIAL_SRC_GENERAL, "Init ph module...");
    ph_init();
}
