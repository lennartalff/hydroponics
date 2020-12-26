# -*- coding: utf-8 -*-
import logging
import crcmod.predefined
import avrhydroponics.msg
logger = logging.getLogger("pkt")
logger.setLevel(logging.WARNING)
ch = logging.StreamHandler()
ch.setLevel(logging.WARNING)
logger.addHandler(ch)

(PACKET_ID_LOGGING, PACKET_ID_CMD_OWI_SET_RES, PACKET_ID_CMD_OWI_GET_RES,
 PACKET_ID_CMD_OWI_MEASURE, PACKET_ID_DATA_OWI, PACKET_ID_RESPONSE_OWI_GET_RES,
 PACKET_ID_CMD_EC_MEASURE, PACKET_ID_CMD_EC_GET_CALIB_FORMAT,
 PACKET_ID_CMD_EC_IMPORT_CALIB, PACKET_ID_CMD_EC_EXPORT_CALIB,
 PACKET_ID_CMD_EC_CLEAR_CALIB, PACKET_ID_CMD_EC_CALIB_DRY,
 PACKET_ID_CMD_EC_CALIB_LOW, PACKET_ID_CMD_EC_CALIB_HIGH,
 PACKET_ID_CMD_EC_COMPENSATION, PACKET_ID_DATA_EC,
 PACKET_ID_RESPONSE_EC_GET_CALIB_FORMAT, PACKET_ID_RESPONSE_EC_EXPORT_CALIB,
 PACKET_ID_CMD_PH_MEASURE, PACKET_ID_CMD_PH_GET_CALIB_FORMAT,
 PACKET_ID_CMD_PH_IMPORT_CALIB, PACKET_ID_CMD_PH_EXPORT_CALIB,
 PACKET_ID_CMD_PH_CLEAR_CALIB, PACKET_ID_CMD_PH_CALIB_LOW,
 PACKET_ID_CMD_PH_CALIB_MID, PACKET_ID_CMD_PH_CALIB_HIGH,
 PACKET_ID_CMD_PH_COMPENSATION, PACKET_ID_DATA_PH,
 PACKET_ID_RESPONSE_PH_GET_CALIB_FORMAT, PACKET_ID_RESPONSE_PH_EXPORT_CALIB,
 PACKET_ID_CMD_LIGHT_SET, PACKET_ID_CMD_LIGHT_GET, PACKET_ID_RESPONSE_LIGHT_GET,
 PACKET_ID_CMD_LIGHT_BLUE_SET, PACKET_ID_CMD_LIGHT_BLUE_GET,
 PACKET_ID_RESPONSE_LIGHT_BLUE_GET, PACKET_ID_CMD_LIGHT_RED_SET,
 PACKET_ID_CMD_LIGHT_RED_GET, PACKET_ID_RESPONSE_LIGHT_RED_GET,
 PACKET_ID_CMD_LIGHT_WHITE_SET, PACKET_ID_CMD_LIGHT_WHITE_GET,
 PACKET_ID_RESPONSE_LIGHT_WHITE_GET, PACKET_ID_CMD_FAN_SET_SPEED,
 PACKET_ID_CMD_FAN_GET_SPEED, PACKET_ID_RESPONSE_FAN_GET_SPEED,
 PACKET_ID_READY_REQUEST, PACKET_ID_RESPONSE_READY_REQUEST,
 PACKET_ID_ACK) = range(48)

crc_fun = crcmod.predefined.mkCrcFun("xmodem")


def cobs_decode(data):
    output = []
    index = 1
    offset = data[0] - 1
    while index < len(data):
        if offset == 0:
            output.append(0)
            offset = data[index]
        else:
            output.append(data[index])
        index = index + 1
        offset = offset - 1
    return bytearray(output)


def cobs_encode(data):
    output = [0 for i in range(len(data) + 2)]
    dst_index = 1
    zero_offset = 1
    for src_byte in data:
        if src_byte == 0:
            output[dst_index - zero_offset] = zero_offset
            zero_offset = 1
        else:
            output[dst_index] = src_byte
            zero_offset += 1
        dst_index += 1

    output[dst_index - zero_offset] = zero_offset
    output[dst_index] = 0

    return output


def check_crc(decoded_data):
    if crc_fun(bytearray(decoded_data)) == 0:
        # strip the crc bytes
        return decoded_data[:-2]
    return None


class Packet():
    def __init__(self):
        self.id = 0
        self.packet_length = 0
        self.payload_length = 0
        self.payload = bytearray([])
        self.crc = 0

    def update_lengths(self):
        self.payload_length = len(self.payload)
        self.packet_length = 5 + self.payload_length

    def __repr__(self):
        return "id: {} | packet_length: {} | payload_length: {} | payload: {}".format(
            self.id, self.packet_length, self.payload_length, self.payload)


def decode_logging(packet):
    return str(packet.payload)


def encode_cmd_owi_set_res(res):
    packet = Packet()
    packet.id = PACKET_ID_CMD_OWI_SET_RES
    packet.payload.append(res)
    packet.update_lengths()
    return packet


def encode_cmd_owi_get_res():
    packet = Packet()
    packet.id = PACKET_ID_CMD_OWI_GET_RES
    packet.update_lengths()
    return packet


def encode_cmd_owi_measure():
    packet = Packet()
    packet.id = PACKET_ID_CMD_OWI_MEASURE
    packet.update_lengths()
    return packet


def decode_data_owi(packet):
    rom = packet.payload[0:8]
    temperature = float(packet.payload[8] | (packet.payload[9] << 8)) / 16.0
    return dict(rom=rom, temperature=temperature)


def decode_response_owi_get_res(packet):
    return int(packet.payload[0])


def encode_cmd_ec_measure():
    packet = Packet()
    packet.id = PACKET_ID_CMD_EC_MEASURE
    packet.update_lengths()
    return packet


def encode_cmd_ec_import_calib(calib_data):
    packet = Packet()
    packet.id = PACKET_ID_CMD_EC_IMPORT_CALIB
    packet.payload = bytearray(calib_data)
    packet.update_lengths()
    return packet


def encode_cmd_ec_export_calib():
    packet = Packet()
    packet.id = PACKET_ID_CMD_EC_EXPORT_CALIB
    packet.update_lengths()
    return packet


def encode_cmd_ec_calib_dry():
    packet = Packet()
    packet.id = PACKET_ID_CMD_EC_CALIB_DRY
    packet.update_lengths()
    return packet


def encode_cmd_ec_calib_low():
    packet = Packet()
    packet.id = PACKET_ID_CMD_EC_CALIB_LOW
    packet.update_lengths()
    return packet


def encode_cmd_ec_calib_high():
    packet = Packet()
    packet.id = PACKET_ID_CMD_EC_CALIB_HIGH
    packet.update_lengths()
    return packet


def encode_cmd_ec_compensation(temperature):
    value = int(temperature * 100)
    packet = Packet()
    packet.id = PACKET_ID_CMD_EC_COMPENSATION
    packet.payload.append(value & 0xFF)
    packet.payload.append((value >> 8) & 0xFF)
    packet.payload.append((value >> 16) & 0xFF)
    packet.payload.append((value >> 24) & 0xFF)
    packet.update_lengths()
    return packet


def decode_data_ec(packet):
    ec = int(packet.payload[0] | (packet.payload[1] << 8)
             | (packet.payload[2] << 16) | (packet.payload[3] << 24))
    return ec


def decode_response_ec_get_calib_format(packet):
    n_strings = int(packet.payload[0])
    n_bytes = int(packet.payload[1])
    return dict(n_strings=n_strings, n_bytes=n_bytes)


def decode_response_ec_export_calib(packet):
    return packet.payload


def encode_cmd_ph_measure():
    packet = Packet()
    packet.id = PACKET_ID_CMD_PH_MEASURE
    packet.update_lengths()
    return packet


def encode_cmd_ph_get_calib_format():
    packet = Packet()
    packet.id = PACKET_ID_CMD_PH_GET_CALIB_FORMAT
    packet.update_lengths()
    return packet


def encode_cmd_ph_import_calib(calib_data):
    packet = Packet()
    packet.id = PACKET_ID_CMD_PH_GET_CALIB_FORMAT
    packet.payload = bytearray(calib_data)
    packet.update_lengths()
    return packet


def encode_cmd_ph_export_calib():
    packet = Packet()
    packet.id = PACKET_ID_CMD_PH_EXPORT_CALIB
    packet.update_lengths()
    return packet


def encode_cmd_ph_calib_low():
    packet = Packet()
    packet.id = PACKET_ID_CMD_PH_CALIB_LOW
    packet.update_lengths()
    return packet


def encode_cmd_ph_calib_mid():
    packet = Packet()
    packet.id = PACKET_ID_CMD_PH_CALIB_MID
    packet.update_lengths()
    return packet


def encode_cmd_ph_calib_high():
    packet = Packet()
    packet.id = PACKET_ID_CMD_PH_CALIB_HIGH
    packet.update_lengths()
    return packet


def encode_cmd_ph_compensation(temperature):
    value = int(temperature * 100)
    packet = Packet()
    packet.id = PACKET_ID_CMD_PH_COMPENSATION
    packet.payload.append(value & 0xFF)
    packet.payload.append((value >> 8) & 0xFF)
    packet.payload.append((value >> 16) & 0xFF)
    packet.payload.append((value >> 24) & 0xFF)
    packet.update_lengths()
    return packet


def decode_data_ph(packet):
    ph = float(packet.payload[0] | (packet.payload[1] << 8)
               | (packet.payload[2] << 16)
               | (packet.payload[3] << 24)) / 1000.0
    return ph


def decode_response_ph_get_calib_format(packet):
    n_strings = int(packet.payload[0])
    n_bytes = int(packet.payload[1])
    return dict(n_strings=n_strings, n_bytes=n_bytes)


def decode_response_ph_export_calib(packet):
    return packet.payload


def encode_cmd_light_set(state):
    packet = Packet()
    packet.id = PACKET_ID_CMD_LIGHT_SET
    packet.payload.append(state)
    packet.update_lengths()
    return packet


def encode_cmd_light_get():
    packet = Packet()
    packet.id = PACKET_ID_CMD_LIGHT_GET
    packet.update_lengths()
    return packet


def decode_response_light_get(packet):
    return int(packet.payload[0])


def encode_cmd_light_blue_set(state):
    packet = Packet()
    packet.id = PACKET_ID_CMD_LIGHT_BLUE_SET
    packet.payload.append(state)
    packet.update_lengths()
    return packet


def encode_cmd_light_blue_get():
    packet = Packet()
    packet.id = PACKET_ID_CMD_LIGHT_BLUE_GET
    packet.update_lengths()
    return packet


def decode_response_light_blue_get(packet):
    return int(packet.payload[0])


def encode_cmd_light_red_set(state):
    packet = Packet()
    packet.id = PACKET_ID_CMD_LIGHT_RED_SET
    packet.payload.append(state)
    packet.update_lengths()
    return packet


def encode_cmd_light_red_get():
    packet = Packet()
    packet.id = PACKET_ID_CMD_LIGHT_RED_GET
    packet.update_lengths()
    return packet


def decode_response_light_red_get(packet):
    return int(packet.payload[0])


def encode_cmd_light_white_set(state):
    packet = Packet()
    packet.id = PACKET_ID_CMD_LIGHT_WHITE_SET
    packet.payload.append(state)
    packet.update_lengths()
    return packet


def encode_cmd_light_white_get():
    packet = Packet()
    packet.id = PACKET_ID_CMD_LIGHT_WHITE_GET
    packet.update_lengths()
    return packet


def decode_response_light_white_get(packet):
    return int(packet.payload[0])


def encode_cmd_fan_set_speed(index, speed):
    packet = Packet()
    packet.id = PACKET_ID_CMD_FAN_SET_SPEED
    packet.payload.append(index)
    packet.payload.append(speed & 0xFF)
    packet.payload.append((speed >> 8) & 0xFF)
    packet.update_lengths()
    return packet


def decode_cmd_fan_get_speed(index):
    packet = Packet()
    packet.id = PACKET_ID_CMD_FAN_GET_SPEED
    packet.payload.append(index)
    packet.update_lengths()
    return packet


def decode_response_fan_get_speed(packet):
    index = int(packet.payload[0])
    speed = int(packet.payload[1] | (packet.payload[2] << 8))
    return dict(index=index, speed=speed)


def encode_ready_request():
    packet = Packet()
    packet.id = PACKET_ID_READY_REQUEST
    packet.update_lengths()
    return packet


def encode_ack(ack_id):
    packet = Packet()
    packet.id = PACKET_ID_ACK
    packet.payload[0] = ack_id
    packet.update_lengths()
    return packet


def decode_ack(packet):
    return int(packet.payload[0])


def packet_serialize(packet):
    data = bytearray()
    data.append(packet.id)
    data.append(packet.packet_length)
    data.append(packet.payload_length)
    data.extend(packet.payload)

    crc = crc_fun(data)
    data.append(crc & 0xFF)
    data.append((crc >> 8) & 0xFF)
    return data


def packet_deserialize(data):
    packet = Packet()
    packet.update_lengths()
    logger.debug("Deserializing data: {}".format(data))
    if len(data) < packet.packet_length:
        logger.error(
            "Data has length {} but minimum packet length is {}".format(
                len(data), packet.packet_length))
        return None
    packet.id = int(data[0])
    packet_length = int(data[1])
    payload_length = int(data[2])
    if packet_length != len(data):
        logger.error(
            "Length mismatch. Packet should have length {} but data has length {}."
            .format(packet_length, len(data)))
        return None
    packet.payload = data[3:3 + payload_length]
    packet.update_lengths()
    if packet.payload_length != payload_length:
        logger.error(
            "Length mismatch. Payload should have length {} but has {}".format(
                packet.payload_length, payload_length))
        return None
    packet.crc = int(data[-2] | (data[-1] << 8))
    crc = crc_fun(bytearray(data[:-2]))
    if packet.crc != crc:
        logger.error("Data has CRC: {} but should have {}".format(
            packet.crc, crc))
        return None
    return packet


def read_packet(port):
    packet = None
    while not packet:
        data = port.read_until(chr(0).encode("utf-8"))
        # read timed out
        if (len(data) == 0 or (data[-1] != 0)):
            return None
        # read 0 byte before timeout but data is to short to be valid.
        if len(data) < 4:
            continue
        data = data[:-1]
        decoded_data = cobs_decode(data)
        packet = packet_deserialize(decoded_data)

    return packet


def packet2ros(packet):
    msg = avrhydroponics.msg.Packet()
    msg.id = packet.id
    msg.payload = packet.payload
    msg.packet_length = packet.packet_length
    msg.payload_length = packet.payload_length
    return msg


def ros2packet(msg):
    packet = Packet()
    packet.id = msg.id
    packet.payload = bytearray(msg.payload)
    packet.packet_length = msg.packet_length
    packet.payload_length = msg.payload_length
    return packet