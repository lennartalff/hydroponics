#!/usr/bin/python3
# -*- coding: utf-8 -*-
import logging
import re
import collections
import time
from PyQt5 import QtCore
import pkt

logger = logging.getLogger("receiver")
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)
logger.addHandler(ch)

LOG_LEVELS = {"debug": 1, "info": 2, "warning": 4, "error": 8}
LOG_LEVELS_REVERSE = {v: k for k, v in LOG_LEVELS.items()}
END_OF_FRAME_BYTE = 0      

class Sender(QtCore.QObject):
    sensor_cycle_complete = QtCore.pyqtSignal(object)
    def __init__(self, serial_device, parent=None):
        super().__init__(parent)
        self.port = serial_device
        self.ready_flag = False

        self.data_mutex = QtCore.QMutex()

        self.user_packets = collections.deque([], 10)
        measure_packets = [
            pkt.encode_cmd_owi_measure(),
            pkt.encode_cmd_ec_measure(),
            pkt.encode_cmd_ph_measure()
        ]
        self.measure_request_packets = collections.deque(measure_packets, len(measure_packets))
        self.measure_request_counter = 0

    def _send_packet(self, packet):
        data = pkt.packet_serialize(packet)
        encoded_data = pkt.cobs_encode(data)
        self.port.write(bytearray(encoded_data))
        self.ready_flag = False

    @QtCore.pyqtSlot()
    def on_read_timeout(self):
        # quite normal that no data could be received if device is in ready state
        if self.ready_flag:
            return
        # request a ready response. in normal operation this should not be
        # necessary because the microcontroller sends a ready signal by itself
        # after each operation.
        packet = pkt.encode_ready_request()
        self._send_packet(packet)

    @QtCore.pyqtSlot(object)
    def on_response_ready_request_received(self, packet):
        packet_to_send = None
        self.data_mutex.lock()
        try:
            packet_to_send = self.user_packets.popleft()
        except IndexError:
            packet_to_send = self.measure_request_packets[0]
            self.measure_request_packets.rotate()
            self.measure_request_counter += 1
            if self.measure_request_counter >= len(self.measure_request_packets):
                self.sensor_cycle_complete.emit(self)
                self.measure_request_counter = 0
        finally:
            self._send_packet(packet_to_send)
            self.data_mutex.unlock()

    @QtCore.pyqtSlot()
    def add_packet(self, packet):
        self.data_mutex.lock()
        self.user_packets.append(packet)
        self.data_mutex.unlock()



class Receiver(QtCore.QObject):
    logging_received = QtCore.pyqtSignal(pkt.Packet)
    data_owi_received = QtCore.pyqtSignal(pkt.Packet)
    response_owi_get_res_received = QtCore.pyqtSignal(pkt.Packet)
    data_ec_received = QtCore.pyqtSignal(pkt.Packet)
    response_ec_get_calib_format_received = QtCore.pyqtSignal(pkt.Packet)
    response_ec_export_calib_received = QtCore.pyqtSignal(pkt.Packet)
    data_ph_received = QtCore.pyqtSignal(pkt.Packet)
    response_ph_get_calib_format_received = QtCore.pyqtSignal(pkt.Packet)
    response_ph_export_calib_received = QtCore.pyqtSignal(pkt.Packet)
    response_light_get_received = QtCore.pyqtSignal(pkt.Packet)
    response_light_blue_get_received = QtCore.pyqtSignal(pkt.Packet)
    response_light_red_get_received = QtCore.pyqtSignal(pkt.Packet)
    response_light_white_get_received = QtCore.pyqtSignal(pkt.Packet)
    response_fan_get_speed_received = QtCore.pyqtSignal(pkt.Packet)
    response_ready_request_received = QtCore.pyqtSignal(pkt.Packet)
    ack_received = QtCore.pyqtSignal(pkt.Packet)

    read_timeout = QtCore.pyqtSignal()

    def __init__(self, serial_device, parent=None):
        super(Receiver, self).__init__(parent=parent)
        self.user_send_lock = QtCore.QMutex()
        self.user_packets = collections.deque([], 5)
        self.port = serial_device
        self._send_list = []
        self.ready_flag = False

    def run(self):
        while True:
            packet = self.read_packet()
            if packet:
                self.handle_packet(packet)
            else:
                self.read_timeout.emit()

    def read_packet(self):
        packet = None
        while not packet:
            data = self.port.read_until(chr(0).encode("utf-8"))
            # read timed out
            if (len(data) == 0) or (data[-1] != 0):
                break
            # read 0 byte before timeout but data is to short to be valid.
            if len(data) < 4:
                continue
            data = data[:-1]
            decoded_data = pkt.cobs_decode(data)
            packet = pkt.packet_deserialize(decoded_data)
            if packet is not None:
                break
        return packet

    def send_packet(self, packet):
        self.user_send_lock.lock()
        self.user_packets.append(packet)
        self.user_send_lock.unlock()

    def _send_packet(self, packet):
        data = pkt.packet_serialize(packet)
        encoded_data = pkt.cobs_encode(data)
        self.port.write(bytearray(encoded_data))

    def handle_packet(self, packet):
        if packet.id == pkt.PACKET_ID_LOGGING:
            logger.debug("Received logging packet")
            self.logging_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_DATA_OWI:
            logger.debug("Received owi data packet")
            self.data_owi_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_RESPONSE_OWI_GET_RES:
            logger.debug("Received owi get res response")
            self.response_owi_get_res_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_DATA_EC:
            logger.debug("Received EC data")
            self.data_ec_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_RESPONSE_EC_GET_CALIB_FORMAT:
            logger.debug("Received ec calib format")
            self.response_ec_get_calib_format_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_RESPONSE_EC_EXPORT_CALIB:
            logger.debug("Received ec calib data")
            self.response_ec_export_calib_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_DATA_PH:
            logger.debug("Received ph data")
            self.data_ph_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_RESPONSE_PH_GET_CALIB_FORMAT:
            logger.debug("Received ph calib format")
            self.response_ph_get_calib_format_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_RESPONSE_PH_EXPORT_CALIB:
            logger.debug("Received ph calib data")
            self.response_ph_export_calib_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_RESPONSE_LIGHT_GET:
            logger.debug("Received light state")
            self.response_light_get_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_RESPONSE_LIGHT_BLUE_GET:
            logger.debug("Received light blue state")
            self.response_light_blue_get_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_RESPONSE_LIGHT_RED_GET:
            logger.debug("Received light red state")
            self.response_light_red_get_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_RESPONSE_LIGHT_WHITE_GET:
            logger.debug("Received light white state")
            self.response_light_white_get_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_RESPONSE_FAN_GET_SPEED:
            logger.debug("Received fan speed")
            self.response_fan_get_speed_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_RESPONSE_READY_REQUEST:
            logger.debug("Received ready state")
            self.response_ready_request_received.emit(packet)
        elif packet.id == pkt.PACKET_ID_ACK:
            logger.debug("Received ACK")
            self.ack_received.emit(packet)
        else:
            logger.debug("Received packet with ID {}. No signal emitted.".format(packet.id))


    def _handle_logging_packet(self, packet):
        data_string = pkt.decode_logging(packet)
        print(data_string)
        match = re.search(r"\[(\w*)\]\[(\w*)\](.*)", data_string)
        if match:
            level = LOG_LEVELS[match.group(1).lower()]
            source = match.group(2)
            message = match.group(3)
            timestamp = time.time()
            packet = dict(level=level,
                          source=source,
                          message=message,
                          timestamp=timestamp)
            if callable(self._logging_packet_callback):
                self._logging_packet_callback(packet)
        else:
            print("Could not parse logging packet")
