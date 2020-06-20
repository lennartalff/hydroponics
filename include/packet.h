#ifndef PACKET
#define PACKET

#include <stdint.h>
#include "return.h"

typedef enum {
    PACKET_ID_LOGGING,

    PACKET_ID_CMD_OWI_SET_RES,
    PACKET_ID_CMD_OWI_GET_RES,
    PACKET_ID_CMD_OWI_MEASURE,
    PACKET_ID_DATA_OWI,
    PACKET_ID_RESPONSE_OWI_GET_RES,

    PACKET_ID_CMD_EC_MEASURE,
    PACKET_ID_CMD_EC_GET_CALIB_FORMAT,
    PACKET_ID_CMD_EC_IMPORT_CALIB,
    PACKET_ID_CMD_EC_EXPORT_CALIB,
    PACKET_ID_CMD_EC_CLEAR_CALIB,
    PACKET_ID_CMD_EC_CALIB_DRY,
    PACKET_ID_CMD_EC_CALIB_LOW,
    PACKET_ID_CMD_EC_CALIB_HIGH,
    PACKET_ID_CMD_EC_COMPENSATION,
    PACKET_ID_DATA_EC,
    PACKET_ID_RESPONSE_EC_GET_CALIB_FORMAT,
    PACKET_ID_RESPONSE_EC_EXPORT_CALIB,

    PACKET_ID_CMD_PH_MEASURE,
    PACKET_ID_CMD_PH_GET_CALIB_FORMAT,
    PACKET_ID_CMD_PH_IMPORT_CALIB,
    PACKET_ID_CMD_PH_EXPORT_CALIB,
    PACKET_ID_CMD_PH_CLEAR_CALIB,
    PACKET_ID_CMD_PH_CALIB_LOW,
    PACKET_ID_CMD_PH_CALIB_MID,
    PACKET_ID_CMD_PH_CALIB_HIGH,
    PACKET_ID_CMD_PH_COMPENSATION,
    PACKET_ID_DATA_PH,
    PACKET_ID_RESPONSE_PH_GET_CALIB_FORMAT,
    PACKET_ID_RESPONSE_PH_EXPORT_CALIB,

    PACKET_ID_CMD_LIGHT_SET,
    PACKET_ID_CMD_LIGHT_GET,
    PACKET_ID_RESPONSE_LIGHT_GET,
    PACKET_ID_CMD_LIGHT_BLUE_SET,
    PACKET_ID_CMD_LIGHT_BLUE_GET,
    PACKET_ID_RESPONSE_LIGHT_BLUE_GET,
    PACKET_ID_CMD_LIGHT_RED_SET,
    PACKET_ID_CMD_LIGHT_RED_GET,
    PACKET_ID_RESPONSE_LIGHT_RED_GET,
    PACKET_ID_CMD_LIGHT_WHITE_SET,
    PACKET_ID_CMD_LIGHT_WHITE_GET,
    PACKET_ID_RESPONSE_LIGHT_WHITE_GET,

    PACKET_ID_CMD_FAN_SET_SPEED,
    PACKET_ID_CMD_FAN_GET_SPEED,
    PACKET_ID_RESPONSE_FAN_GET_SPEED,

    PACKET_ID_READY_REQUEST,
    PACKET_ID_RESPONSE_READY_REQUEST,
    PACKET_ID_ACK
}
packet_id_t;

typedef struct{
    uint8_t id;
    uint8_t packet_length;
    uint8_t payload_length;
    uint8_t payload[255];
    uint16_t crc;
} packet_t;

void encode_logging(packet_t *packet, char *string);
void encode_cmd_owi_set_res(packet_t *packet, uint8_t res);
void decode_cmd_owi_set_res(packet_t *packet, uint8_t *res);
void encode_cmd_owi_get_res(packet_t *packet);
void encode_cmd_owi_measure(packet_t *packet);
void encode_data_owi(packet_t *packet, uint8_t *rom, uint16_t temperature);
void decode_data_owi(packet_t *packet, uint8_t *rom, uint16_t *temperature);
void encode_response_owi_get_res(packet_t *packet, uint8_t res);
void decode_response_owi_get_res(packet_t *packet, uint8_t *res);
void encode_cmd_ec_measure(packet_t *packet);
void encode_cmd_ec_import_calib(packet_t *packet, uint8_t *data,
                                uint8_t length);
void encode_cmd_ec_export_calib(packet_t *packet);
void encode_cmd_ec_clear_calib(packet_t *packet);
void encode_cmd_ec_calib_dry(packet_t *packet);
void encode_cmd_ec_calib_low(packet_t *packet);
void encode_cmd_ec_calib_high(packet_t *packet);
void encode_cmd_ec_compensation(packet_t *packet, float temperature);
void decode_cmd_ec_compensation(packet_t *packet, float *temperature);
void encode_data_ec(packet_t *packet, uint32_t value);
void encode_response_ec_get_calib_format(packet_t *packet, uint8_t n_strings,
                                         uint8_t n_bytes);
void encode_response_ec_export_calib(packet_t *packet, uint8_t *data,
                                     uint8_t length);
void encode_cmd_ph_measure(packet_t *packet);
void encode_cmd_ph_get_calib_format(packet_t *packet);
void encode_cmd_ph_import_calib(packet_t *packet, uint8_t *data,
                                uint8_t length);
void encode_cmd_ph_export_calib(packet_t *packet);
void encode_cmd_ph_clear_calib(packet_t *packet);
void encode_cmd_ph_calib_low(packet_t *packet);
void encode_cmd_ph_calib_mid(packet_t *packet);
void encode_cmd_ph_calib_high(packet_t *packet);
void encode_cmd_ph_compensation(packet_t *packet, float temperature);
void decode_cmd_ph_compensation(packet_t *packet, float *temperature);
void encode_data_ph(packet_t *packet, uint32_t data);
void encode_response_ph_get_calib_format(packet_t *packet, uint8_t n_strings,
                                         uint8_t n_bytes);
void encode_response_ph_export_calib(packet_t *packet, uint8_t *data,
                                     uint8_t length);
void encode_cmd_light_set(packet_t *packet, uint8_t state);
void decode_cmd_light_set(packet_t *packet, uint8_t *state);
void encode_cmd_light_get(packet_t *packet);
void encode_response_light_get(packet_t *packet, uint8_t state);
void decode_response_light_get(packet_t *packet, uint8_t *state);
void encode_cmd_light_blue_set(packet_t *packet, uint8_t state);
void decode_cmd_light_blue_set(packet_t *packet, uint8_t *state);
void encode_cmd_light_blue_get(packet_t *packet);
void encode_response_light_blue_get(packet_t *packet, uint8_t state);
void decode_response_light_blue_get(packet_t *packet, uint8_t *state);
void encode_cmd_light_red_set(packet_t *packet, uint8_t state);
void decode_cmd_light_red_set(packet_t *packet, uint8_t *state);
void encode_cmd_light_red_get(packet_t *packet);
void encode_response_light_red_get(packet_t *packet, uint8_t state);
void decode_reponse_light_red_get(packet_t *packet, uint8_t *state);
void encode_cmd_light_white_set(packet_t *packet, uint8_t state);
void decode_cmd_light_white_set(packet_t *packet, uint8_t *state);
void encode_cmd_light_white_get(packet_t *packet);
void encode_response_light_white_get(packet_t *packet, uint8_t state);
void decode_response_light_white_get(packet_t *packet, uint8_t *state);
void encode_cmd_fan_set_speed(packet_t *packet, uint8_t index, uint16_t speed);
void decode_cmd_fan_set_speed(packet_t *packet, uint8_t *index,
                              uint16_t *speed);
void encode_cmd_fan_get_speed(packet_t *packet, uint8_t index);
void decode_cmd_fan_get_speed(packet_t *packet, uint8_t *index);
void encode_response_fan_get_speed(packet_t *packet, uint8_t index,
                                   uint16_t speed);
void decode_response_fan_get_speed(packet_t *packet, uint8_t *index,
                                   uint16_t *speed);
void encode_ready_request(packet_t *packet);
void encode_response_ready_request(packet_t *packet);
void encode_ack(packet_t *packet, uint8_t ack_id);
void decode_ack(packet_t *packet, uint8_t *ack_id);
uint8_t packet_serialize(packet_t *packet, uint8_t *serialized_data);
return_status_t packet_deserialize(packet_t *packet, uint8_t *serialized_data,
                                   uint8_t length);

#endif /* PACKET */
