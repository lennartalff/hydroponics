#include <packet.h>

#define OWI_ROM_SIZE 8

static uint8_t compute_packet_length(packet_t *packet) {
    return sizeof(packet->id) + sizeof(packet->packet_length) +
           sizeof(packet->payload_length) + packet->payload_length +
           sizeof(packet->crc);
}

uint16_t crc_xmodem(uint8_t *data, uint8_t length) {
    uint16_t crc = 0;
    for (uint8_t i = 0; i < length; i++) {
        crc = crc ^ ((uint16_t)data[i] << 8);
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void encode_logging(packet_t *packet, char *string) {
    packet->id = PACKET_ID_LOGGING;
    uint8_t i = 0;
    while (string[i] != '\0') {
        packet->payload[i] = string[i];
        i++;
    }
    packet->payload_length = i;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_owi_set_res(packet_t *packet, uint8_t res) {
    packet->id = PACKET_ID_CMD_OWI_SET_RES;
    packet->payload[0] = res;
    packet->payload_length = sizeof(res);
    packet->packet_length = compute_packet_length(packet);
}

void decode_cmd_owi_set_res(packet_t *packet, uint8_t *res) {
    *res = packet->payload[0];
}

void encode_cmd_owi_get_res(packet_t *packet) {
    packet->id = PACKET_ID_CMD_OWI_GET_RES;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_owi_measure(packet_t *packet) {
    packet->id = PACKET_ID_CMD_OWI_MEASURE;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_data_owi(packet_t *packet, uint8_t *rom, uint16_t temperature) {
    packet->id = PACKET_ID_DATA_OWI;
    for (uint8_t i = 0; i < OWI_ROM_SIZE; i++) {
        packet->payload[i] = rom[i];
    }
    packet->payload[OWI_ROM_SIZE] = (uint8_t)(temperature & 0xFF);
    packet->payload[OWI_ROM_SIZE + 1] = (uint8_t)((temperature >> 8) & 0xFF);
    packet->payload_length = OWI_ROM_SIZE + sizeof(temperature);
    packet->packet_length = compute_packet_length(packet);
}

void decode_data_owi(packet_t *packet, uint8_t *rom, uint16_t *temperature) {
    for (uint8_t i = 0; i < OWI_ROM_SIZE; i++) {
        rom[i] = packet->payload[i];
    }
    *temperature = (uint16_t)packet->payload[OWI_ROM_SIZE] |
                   (packet->payload[OWI_ROM_SIZE + 1] << 8);
}

void encode_response_owi_get_res(packet_t *packet, uint8_t res) {
    packet->id = PACKET_ID_RESPONSE_OWI_GET_RES;
    packet->payload[0] = res;
    packet->payload_length = sizeof(res);
    packet->packet_length = compute_packet_length(packet);
}

void decode_response_owi_get_res(packet_t *packet, uint8_t *res) {
    *res = packet->payload[0];
}

void encode_cmd_ec_measure(packet_t *packet) {
    packet->id = PACKET_ID_CMD_EC_MEASURE;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ec_import_calib(packet_t *packet, uint8_t *data,
                                uint8_t length) {
    packet->id = PACKET_ID_CMD_EC_IMPORT_CALIB;
    for (uint8_t i = 0; i < length; i++) {
        packet->payload[i] = data[i];
    }
    packet->payload_length = length;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ec_export_calib(packet_t *packet) {
    packet->id = PACKET_ID_CMD_EC_EXPORT_CALIB;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ec_clear_calib(packet_t *packet) {
    packet->id = PACKET_ID_CMD_EC_CLEAR_CALIB;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ec_calib_dry(packet_t *packet) {
    packet->id = PACKET_ID_CMD_EC_CALIB_DRY;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ec_calib_low(packet_t *packet) {
    packet->id = PACKET_ID_CMD_EC_CALIB_LOW;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ec_calib_high(packet_t *packet) {
    packet->id = PACKET_ID_CMD_EC_CALIB_HIGH;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ec_compensation(packet_t *packet, float temperature) {
    uint32_t value = (uint32_t)(temperature * 100);
    packet->id = PACKET_ID_CMD_EC_COMPENSATION;
    packet->payload[0] = (uint8_t)(value & 0xFF);
    packet->payload[1] = (uint8_t)((value >> 8) & 0xFF);
    packet->payload[2] = (uint8_t)((value >> 16) & 0xFF);
    packet->payload[3] = (uint8_t)((value >> 24) & 0xFF);
    packet->packet_length = sizeof(value);
    packet->packet_length = compute_packet_length(packet);
}

void decode_cmd_ec_compensation(packet_t *packet, float *temperature) {
    uint32_t value = 0;
    value = packet->payload[0] | ((uint32_t)packet->payload[1] << 8) |
            ((uint32_t)packet->payload[2] << 16) | ((uint32_t)packet->payload[3] << 24);
    *temperature = (float)value / 100.0;
}

void encode_data_ec(packet_t *packet, uint32_t value) {
    packet->id = PACKET_ID_DATA_EC;
    packet->payload[0] = (uint8_t)(value & 0xFF);
    packet->payload[1] = (uint8_t)((value >> 8) & 0xFF);
    packet->payload[2] = (uint8_t)((value >> 16) & 0xFF);
    packet->payload[3] = (uint8_t)((value >> 24) & 0xFF);
    packet->payload_length = sizeof(value);
    packet->packet_length = compute_packet_length(packet);
}

void encode_response_ec_get_calib_format(packet_t *packet, uint8_t n_strings,
                                         uint8_t n_bytes) {
    packet->id = PACKET_ID_RESPONSE_EC_GET_CALIB_FORMAT;
    packet->payload[0] = n_strings;
    packet->payload[1] = n_bytes;
    packet->payload_length = sizeof(n_strings) + sizeof(n_bytes);
    packet->packet_length = compute_packet_length(packet);
}

void encode_response_ec_export_calib(packet_t *packet, uint8_t *data,
                                     uint8_t length) {
    packet->id = PACKET_ID_RESPONSE_EC_EXPORT_CALIB;
    for (uint8_t i = 0; i < length; i++) {
        packet->payload[i] = data[i];
    }
    packet->payload_length = length;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ph_measure(packet_t *packet) {
    packet->id = PACKET_ID_CMD_PH_MEASURE;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ph_get_calib_format(packet_t *packet) {
    packet->id = PACKET_ID_CMD_PH_GET_CALIB_FORMAT;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ph_import_calib(packet_t *packet, uint8_t *data,
                                uint8_t length) {
    packet->id = PACKET_ID_CMD_PH_IMPORT_CALIB;
    for (uint8_t i = 0; i < length; i++) {
        packet->payload[i] = data[i];
    }
    packet->payload_length = length;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ph_export_calib(packet_t *packet) {
    packet->id = PACKET_ID_CMD_PH_EXPORT_CALIB;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ph_clear_calib(packet_t *packet) {
    packet->id = PACKET_ID_CMD_PH_CLEAR_CALIB;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ph_calib_low(packet_t *packet) {
    packet->id = PACKET_ID_CMD_PH_CALIB_LOW;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ph_calib_mid(packet_t *packet) {
    packet->id = PACKET_ID_CMD_PH_CALIB_MID;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}
void encode_cmd_ph_calib_high(packet_t *packet) {
    packet->id = PACKET_ID_CMD_PH_CALIB_HIGH;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_ph_compensation(packet_t *packet, float temperature) {
    uint32_t value = (uint32_t)(temperature * 100);
    packet->id = PACKET_ID_CMD_PH_COMPENSATION;
    packet->payload[0] = (uint8_t)(value & 0xFF);
    packet->payload[1] = (uint8_t)((value >> 8) & 0xFF);
    packet->payload[2] = (uint8_t)((value >> 16) & 0xFF);
    packet->payload[3] = (uint8_t)((value >> 24) & 0xFF);
    packet->packet_length = sizeof(value);
    packet->packet_length = compute_packet_length(packet);
}

void decode_cmd_ph_compensation(packet_t *packet, float *temperature) {
    uint32_t value = 0;
    value = packet->payload[0] | ((uint32_t)packet->payload[1] << 8) |
            ((uint32_t)packet->payload[2] << 16) | ((uint32_t)packet->payload[3] << 24);
    *temperature = (float)value / 100.0;
}

void encode_data_ph(packet_t *packet, uint32_t data) {
    packet->id = PACKET_ID_DATA_PH;
    packet->payload[0] = (uint8_t)(data & 0xFF);
    packet->payload[1] = (uint8_t)((data >> 8) & 0xFF);
    packet->payload[2] = (uint8_t)((data >> 16) & 0xFF);
    packet->payload[3] = (uint8_t)((data >> 24) & 0xFF);
    packet->payload_length = sizeof(data);
    packet->packet_length = compute_packet_length(packet);
}

void encode_response_ph_get_calib_format(packet_t *packet, uint8_t n_strings,
                                         uint8_t n_bytes) {
    packet->id = PACKET_ID_RESPONSE_PH_GET_CALIB_FORMAT;
    packet->payload[0] = n_strings;
    packet->payload[1] = n_bytes;
    packet->payload_length = sizeof(n_strings) + sizeof(n_bytes);
    packet->packet_length = compute_packet_length(packet);
}

void encode_response_ph_export_calib(packet_t *packet, uint8_t *data,
                                     uint8_t length) {
    packet->id = PACKET_ID_RESPONSE_PH_EXPORT_CALIB;
    for (uint8_t i = 0; i < length; i++) {
        packet->payload[i] = data[i];
    }
    packet->payload_length = length;
    packet->packet_length = compute_packet_length(packet);
}

void encode_cmd_light_set(packet_t *packet, uint8_t state) {
    packet->id = PACKET_ID_CMD_LIGHT_SET;
    packet->payload[0] = state;
    packet->payload_length = sizeof(state);
    packet->packet_length = compute_packet_length(packet);
}

void decode_cmd_light_set(packet_t *packet, uint8_t *state) {
    *state = packet->payload[0];
}

void encode_cmd_light_get(packet_t *packet) {
    packet->id = PACKET_ID_CMD_LIGHT_GET;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_response_light_get(packet_t *packet, uint8_t state) {
    packet->id = PACKET_ID_RESPONSE_LIGHT_GET;
    packet->payload[0] = state;
    packet->payload_length = sizeof(state);
    packet->packet_length = compute_packet_length(packet);
}

void decode_response_light_get(packet_t *packet, uint8_t *state) {
    *state = packet->payload[0];
}

void encode_cmd_light_blue_set(packet_t *packet, uint8_t state) {
    packet->id = PACKET_ID_CMD_LIGHT_BLUE_SET;
    packet->payload[0] = state;
    packet->payload_length = sizeof(state);
    packet->packet_length = compute_packet_length(packet);
}

void decode_cmd_light_blue_set(packet_t *packet, uint8_t *state) {
    *state = packet->payload[0];
}

void encode_cmd_light_blue_get(packet_t *packet) {
    packet->id = PACKET_ID_CMD_LIGHT_BLUE_GET;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_response_light_blue_get(packet_t *packet, uint8_t state) {
    packet->id = PACKET_ID_RESPONSE_LIGHT_BLUE_GET;
    packet->payload[0] = state;
    packet->payload_length = sizeof(state);
    packet->packet_length = compute_packet_length(packet);
}

void decode_response_light_blue_get(packet_t *packet, uint8_t *state) {
    *state = packet->payload[0];
}

void encode_cmd_light_red_set(packet_t *packet, uint8_t state) {
    packet->id = PACKET_ID_CMD_LIGHT_RED_SET;
    packet->payload[0] = state;
    packet->packet_length = sizeof(state);
    packet->packet_length = compute_packet_length(packet);
}

void decode_cmd_light_red_set(packet_t *packet, uint8_t *state) {
    *state = packet->payload[0];
}

void encode_cmd_light_red_get(packet_t *packet) {
    packet->id = PACKET_ID_CMD_LIGHT_RED_GET;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_response_light_red_get(packet_t *packet, uint8_t state) {
    packet->id = PACKET_ID_RESPONSE_LIGHT_RED_GET;
    packet->payload[0] = state;
    packet->payload_length = sizeof(state);
    packet->packet_length = compute_packet_length(packet);
}

void decode_reponse_light_red_get(packet_t *packet, uint8_t *state) {
    *state = packet->payload[0];
}

void encode_cmd_light_white_set(packet_t *packet, uint8_t state) {
    packet->id = PACKET_ID_CMD_LIGHT_WHITE_SET;
    packet->payload[0] = state;
    packet->payload_length = sizeof(state);
    packet->packet_length = compute_packet_length(packet);
}

void decode_cmd_light_white_set(packet_t *packet, uint8_t *state) {
    *state = packet->payload[0];
}

void encode_cmd_light_white_get(packet_t *packet) {
    packet->id = PACKET_ID_CMD_LIGHT_WHITE_GET;
    packet->packet_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_response_light_white_get(packet_t *packet, uint8_t state) {
    packet->id = PACKET_ID_RESPONSE_LIGHT_WHITE_GET;
    packet->payload[0] = state;
    packet->payload_length = sizeof(state);
    packet->packet_length = compute_packet_length(packet);
}

void decode_response_light_white_get(packet_t *packet, uint8_t *state) {
    *state = packet->payload[0];
}

void encode_cmd_fan_set_speed(packet_t *packet, uint8_t index, uint16_t speed) {
    packet->id = PACKET_ID_CMD_FAN_SET_SPEED;
    packet->payload[0] = index;
    packet->payload[1] = (uint8_t)(speed & 0xFF);
    packet->payload[2] = (uint8_t)((speed >> 8) & 0xFF);
    packet->payload_length = sizeof(index) + sizeof(speed);
    packet->packet_length = compute_packet_length(packet);
}

void decode_cmd_fan_set_speed(packet_t *packet, uint8_t *index,
                              uint16_t *speed) {
    *index = packet->payload[0];
    *speed = (uint16_t)packet->payload[1] | (packet->payload[2] << 8);
}

void encode_cmd_fan_get_speed(packet_t *packet, uint8_t index) {
    packet->id = PACKET_ID_CMD_FAN_GET_SPEED;
    packet->payload[0] = index;
    packet->payload_length = sizeof(index);
    packet->packet_length = compute_packet_length(packet);
}

void decode_cmd_fan_get_speed(packet_t *packet, uint8_t *index) {
    *index = packet->payload[0];
}

void encode_response_fan_get_speed(packet_t *packet, uint8_t index,
                                   uint16_t speed) {
    packet->id = PACKET_ID_RESPONSE_FAN_GET_SPEED;
    packet->payload[0] = index;
    packet->payload[1] = (uint8_t)(speed & 0xFF);
    packet->payload[2] = (uint8_t)((speed >> 8) & 0xFF);
    packet->payload_length = sizeof(index) + sizeof(speed);
    packet->packet_length = compute_packet_length(packet);
}

void decode_response_fan_get_speed(packet_t *packet, uint8_t *index,
                                   uint16_t *speed) {
    *index = packet->payload[0];
    *speed = (uint16_t)packet->payload[1] | (packet->payload[2] << 8);
}

void encode_ready_request(packet_t *packet) {
    packet->id = PACKET_ID_READY_REQUEST;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_response_ready_request(packet_t *packet) {
    packet->id = PACKET_ID_RESPONSE_READY_REQUEST;
    packet->payload_length = 0;
    packet->packet_length = compute_packet_length(packet);
}

void encode_ack(packet_t *packet, uint8_t ack_id) {
    packet->id = PACKET_ID_ACK;
    packet->payload[0] = ack_id;
    packet->payload_length = sizeof(ack_id);
    packet->packet_length = compute_packet_length(packet);
}

void decode_ack(packet_t *packet, uint8_t *ack_id) {
    *ack_id = packet->payload[0];
}

uint8_t packet_serialize(packet_t *packet, uint8_t *serialized_data) {
    uint16_t crc;
    serialized_data[0] = packet->id;
    serialized_data[1] = packet->packet_length;
    serialized_data[2] = packet->payload_length;
    for (uint8_t i = 0; i < packet->payload_length; i++) {
        serialized_data[i + 3] = packet->payload[i];
    }
    crc = crc_xmodem(serialized_data,
                     packet->packet_length - sizeof(packet->crc));
    serialized_data[packet->packet_length - sizeof(packet->crc)] =
        (uint8_t)crc & 0xFF;
    serialized_data[packet->packet_length - sizeof(packet->crc) + 1] =
        (uint8_t)((crc >> 8) & 0xFF);
    return packet->packet_length;
}

return_status_t packet_deserialize(packet_t *packet, uint8_t *serialized_data,
                                   uint8_t length) {
    uint8_t crc_offset;
    uint16_t crc;
    packet->payload_length = 0;
    // compute length of a minimal length packet and compare to length of
    // serialized_data
    if (compute_packet_length(packet) > length) {
        return RET_PACKET_LENGTH_MISMATCH;
    }
    packet->id = serialized_data[0];
    packet->packet_length = serialized_data[1];
    if (packet->packet_length != length) {
        return RET_PACKET_LENGTH_MISMATCH;
    }
    packet->payload_length = serialized_data[2];
    for (uint8_t i = 0; i < packet->payload_length; i++) {
        packet->payload[i] = serialized_data[i + 3];
    }
    crc_offset = packet->packet_length - sizeof(packet->crc);
    packet->crc = (uint16_t)serialized_data[crc_offset] |
                  ((uint16_t)serialized_data[crc_offset + 1] << 8);

    if (crc_xmodem(serialized_data, length - sizeof(crc)) != packet->crc) {
        return RET_PACKET_CRC_ERR;
    }

    return RET_SUCCESS;
}
