#ifndef PACKET_HANDLER
#define PACKET_HANDLER

#include "serial.h"
#include "packet.h"

void handle_cmd_owi_set_res(packet_t *packet);
void handle_cmd_owi_get_res(packet_t *packet);
void handle_cmd_owi_measure(packet_t *packet);
void handle_cmd_ec_measure(packet_t *packet);
void handle_cmd_ec_import_calib(packet_t *packet);
void handle_cmd_ec_export_calib(packet_t *packet);
void handle_cmd_ec_clear_calib(packet_t *packet);
void handle_cmd_ec_calib_dry(packet_t *packet);
void handle_cmd_ec_calib_low(packet_t *packet);
void handle_cmd_ec_calib_high(packet_t *packet);
void handle_cmd_ec_compensation(packet_t *packet);
void handle_cmd_ph_measure(packet_t *packet);
void handle_cmd_ph_import_calib(packet_t *packet);
void handle_cmd_ph_export_calib(packet_t *packet);
void handle_cmd_ph_clear_calib(packet_t *packet);
void handle_cmd_ph_calib_low(packet_t *packet);
void handle_cmd_ph_calib_mid(packet_t *packet);
void handle_cmd_ph_calib_high(packet_t *packet);
void handle_cmd_ph_compensation(packet_t *packet);
void handle_cmd_light_set(packet_t *packet);
void handle_cmd_light_get(packet_t *packet);
void handle_cmd_light_blue_set(packet_t *packet);
void handle_cmd_light_blue_get(packet_t *packet);
void handle_cmd_light_red_set(packet_t *packet);
void handle_cmd_light_red_get(packet_t *packet);
void handle_cmd_light_white_set(packet_t *packet);
void handle_cmd_light_white_get(packet_t *packet);
void handle_cmd_fan_set_speed(packet_t *packet);
void handle_cmd_fan_get_speed(packet_t *packet);
void handle_cmd_unknown(packet_t *packet);

#endif /* PACKET_HANDLER */
