import time
from PyQt5 import QtCore
import pkt
import threading
import copy

LED_TEMPERATURE_ROMS = [
    [40, 160, 118, 196, 10, 0, 0, 238],
    [40, 204, 21, 196, 10, 0, 0, 15],
    [40, 182, 213, 195, 10, 0, 0, 252],
    [40, 174, 48, 196, 10, 0, 0, 136],
    [40, 89, 181, 195, 10, 0, 0, 224],
    [40, 101, 142, 196, 10, 0, 0, 2],
    [40, 107, 181, 196, 10, 0, 0, 229],
    [40, 103, 111, 196, 10, 0, 0, 123],
]


class SensorCtrl(QtCore.QObject):
    new_led_temperature = QtCore.pyqtSignal(float, float, float)
    new_water_temperature = QtCore.pyqtSignal(float)
    new_ec_data = QtCore.pyqtSignal(int)
    new_ph_data = QtCore.pyqtSignal(float)

    def __init__(self, parent=None):
        super().__init__(parent=parent)
        self.data_mutex = QtCore.QMutex()
        self.ds18b20_list = []
        self.water_temperature = {}
        self.ec = {}
        self.ph = {}

    @QtCore.pyqtSlot(object)
    def on_ec_data(self, packet):
        self.data_mutex.lock()
        self.ec["value"] = pkt.decode_data_ec(packet)
        self.ec["timestamp"] = time.time()
        self.new_ec_data.emit(self.ec["value"])
        self.data_mutex.unlock()

    @QtCore.pyqtSlot(object)
    def on_ph_data(self, packet):
        self.data_mutex.lock()
        self.ph["value"] = pkt.decode_data_ph(packet)
        self.ph["timestamp"] = time.time()
        self.new_ph_data.emit(self.ph["value"])
        self.data_mutex.unlock()

    @QtCore.pyqtSlot(object)
    def on_owi_data(self, packet):
        self.data_mutex.lock()
        data = pkt.decode_data_owi(packet)
        index = self._find_dict_index(self.ds18b20_list, "rom", data["rom"])

        entry = {}
        entry["rom"] = list(data["rom"])
        entry["value"] = data["temperature"]
        entry["timestamp"] = time.time()

        if index is None:
            self.ds18b20_list.append(entry)
        else:
            self.ds18b20_list[index] = entry

        if entry["rom"] in LED_TEMPERATURE_ROMS:
            min_val, max_val, avg_val = self._led_min_max_avg()
            self.new_led_temperature.emit(min_val, max_val, avg_val)
        else:
            avg_value = self._water_temperature_avg()
            self.water_temperature["timestamp"] = time.time()
            self.water_temperature["value"] = avg_value
            self.new_water_temperature(avg_value)
        self.data_mutex.unlock()

    def get_water_temperature(self):
        self.data_mutex.lock()
        try:
            temperature = copy.deepcopy(self.water_temperature["value"])
        except KeyError:
            temperature = None
        self.data_mutex.unlock()
        return temperature

    @QtCore.pyqtSlot(object)
    def on_measurement_cycle_complete(self, sender):
        temperature = self.get_water_temperature()
        if temperature is None:
            return
        packet = pkt.encode_cmd_ec_compensation(temperature)
        sender.


    def _find_dict_index(self, dict_list, key, value):
        return next(
            (i for i, item in enumerate(dict_list) if item[key] == value),
            None)

    def _led_min_max_avg(self):
        led_list = self._get_led_list()
        sum_value = 0
        min_value = led_list[0]["value"]
        max_value = min_value
        for item in led_list:
            sum_value += item["value"]
            if item["value"] < min_value:
                min_value = item["value"]
            if item["value"] > max_value:
                max_value = item["value"]
        avg_value = float(sum_value) / len(led_list)
        return min_value, max_value, avg_value

    def _water_temperature_avg(self):
        water_list = self._get_water_list()
        sum_value = 0.0
        for item in water_list:
            sum_value += item["value"]
        avg_value = float(sum_value) / len(water_list)
        return avg_value

    def _get_water_list(self):
        water_list = []
        for item in self.ds18b20_list:
            if item["rom"] not in LED_TEMPERATURE_ROMS:
                water_list.append(item)

        return water_list

    def _get_led_list(self):
        led_list = []
        for item in self.ds18b20_list:
            if item["rom"] in LED_TEMPERATURE_ROMS:
                led_list.append(item)

        return led_list
