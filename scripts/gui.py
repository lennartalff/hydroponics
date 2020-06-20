#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
import logging
import datetime
import time
import numpy as np
from PyQt5.QtCore import Qt, QObject, pyqtSignal, QThread
from PyQt5 import QtCore
from PyQt5 import QtWidgets, QtGui
import matplotlib
matplotlib.use("Qt5Agg")
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
import matplotlib.dates as mpldates
import serial
import receiver
import pkt
import sensor

logger = logging.getLogger("gui")
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)
logger.addHandler(ch)


class MainWindow(QtWidgets.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Test")


        layout = QtWidgets.QVBoxLayout()
        self.tab_widget = QtWidgets.QTabWidget(self)
        layout.addWidget(self.tab_widget)
        atmega_console = self.create_atmega_console_widget()
        self.tab_widget.addTab(atmega_console, "Console")
        self.tab_widget.addTab(CustomPlot(self), "Plot")

        widget = OverviewWidget(parent=self)
        self.tab_widget.addTab(widget, "Overview")

        self.port = serial.Serial("/dev/ftdi", baudrate=250000, timeout=2)
        self.receiver = receiver.Receiver(self.port)
        self.receiver_thread = QtCore.QThread()
        self.receiver.moveToThread(self.receiver_thread)
        self.receiver_thread.started.connect(self.receiver.run)

        self.sender = receiver.Sender(self.port)
        self.sender_thread = QtCore.QThread()
        self.sender.moveToThread(self.sender_thread)
        self.receiver.response_ready_request_received.connect(self.sender.on_response_ready_request_received)
        self.receiver.read_timeout.connect(self.sender.on_read_timeout)

        self.receiver_thread.start()
        self.sender_thread.start()

        self.sensor_controller = sensor.SensorCtrl(self)
        self.receiver.data_owi_received.connect(self.sensor_controller.on_owi_data)
        self.receiver.data_ph_received.connect(self.sensor_controller.on_ph_data)
        self.receiver.data_ec_received.connect(self.sensor_controller.on_ec_data)
        self.sensor_controller.new_led_temperature.connect(widget.set_led_temperature)
        self.sensor_controller.new_ec_data.connect(widget.set_ec_value)
        self.sensor_controller.new_ph_data.connect(widget.set_ph_value)
        self.sensor_con


        self.setLayout(layout)
        self.showMaximized()
        # self.receiver_thread.start()
        # self.sender_thread.start()

    def create_atmega_console_widget(self):
        # create widget
        atmega_console = AtMegaConsole(self)
        # set logging packet callback to emit signal when packet arrives
        return atmega_console

class AtMegaConsole(QtWidgets.QWidget):
    LOG_COUNT_LIMIT = 100

    log_received = pyqtSignal(dict)

    def __init__(self, parent=None):
        self.logger = logger

        self._item_count = {}
        for key in receiver.LOG_LEVELS:
            self._item_count[key] = 0

        super().__init__(parent)
        layout = QtWidgets.QVBoxLayout()
        self.console = self.create_console_widget()
        self.filter_widget = self.create_filter_widget()
        self.status_widget = self.create_status_widget()

        self.delete_button = QtWidgets.QPushButton("Delete")
        self.delete_button.clicked.connect(self.on_delete_button_clicked)

        self.update_status_widget()

        hlayout = QtWidgets.QHBoxLayout()
        hlayout.addWidget(self.filter_widget)
        hlayout.addWidget(self.delete_button)
        layout.addLayout(hlayout)
        layout.addWidget(self.console)
        layout.addWidget(self.status_widget)
        self.setLayout(layout)

        # self.create_test_logs()

        self.log_received.connect(self._on_log_received)

    def create_test_logs(self):
        self.add_log_entry({
            "level": 4,
            "source": "ph",
            "timestamp": "somewhen",
            "message": "not so good"
        })
        self.add_log_entry({
            "level": 1,
            "source": "css",
            "timestamp": "now",
            "message": "hello"
        })
        self.add_log_entry({
            "level": 8,
            "source": "ec",
            "timestamp": "later",
            "message": "ARGH!"
        })

    def _on_log_received(self, log):
        self.add_log_entry(log)

    def on_delete_button_clicked(self):
        items = self.console.selectedItems()
        for item in items:
            index = self.console.indexOfTopLevelItem(item)
            self.console.takeTopLevelItem(index)

    def create_filter_widget(self):
        filter_widget = QtWidgets.QComboBox(self)
        filter_widget.setObjectName("Filter")
        filter_widget.addItems(("Debug", "Info", "Warning", "Error"))
        filter_widget.currentIndexChanged.connect(self.on_filter_change)
        return filter_widget

    def create_status_widget(self):
        widget = QtWidgets.QStatusBar(self)
        widget.setObjectName("Status")
        widget.setStyleSheet(
            "QtWidgets.QStatusbar QtWidgets.QLabel { border: 1px solid red; border-radius: 3px; }"
        )
        widget.debug_label = QtWidgets.QLabel("", widget)
        widget.addWidget(widget.debug_label)
        widget.debug_label.setFrameStyle(QtWidgets.QFrame.Panel
                                         | QtWidgets.QFrame.Sunken)
        widget.info_label = QtWidgets.QLabel("", widget)
        widget.addWidget(widget.info_label)
        widget.info_label.setFrameStyle(QtWidgets.QFrame.Panel
                                        | QtWidgets.QFrame.Sunken)
        widget.warning_label = QtWidgets.QLabel("", widget)
        widget.addWidget(widget.warning_label)
        widget.warning_label.setFrameStyle(QtWidgets.QFrame.Panel
                                           | QtWidgets.QFrame.Sunken)
        widget.error_label = QtWidgets.QLabel("", widget)
        widget.addWidget(widget.error_label)
        widget.error_label.setFrameStyle(QtWidgets.QFrame.Panel
                                         | QtWidgets.QFrame.Sunken)
        return widget

    def update_status_widget(self):
        self.status_widget.debug_label.setText("Debug: {:3d}".format(
            self._item_count["debug"]))
        self.status_widget.info_label.setText("Info: {:3d}".format(
            self._item_count["info"]))
        self.status_widget.warning_label.setText("Warning: {:3d}".format(
            self._item_count["warning"]))
        self.status_widget.error_label.setText("Error: {:3d}".format(
            self._item_count["error"]))

    def apply_filter(self, level):
        self.logger.debug("Applying filter with level {}".format(level))
        if level is None:
            return
        if isinstance(level, str):
            level = receiver.LOG_LEVELS[level]
        console = self.console
        iterator = QtWidgets.QTreeWidgetItemIterator(console)
        while iterator.value():
            item = iterator.value()
            item.setHidden(item.severity_level < level)
            iterator += 1

    def get_filter_level(self):
        filter_widget = self.filter_widget
        if not filter_widget:
            return receiver.LOG_LEVELS["debug"]
        level_str = filter_widget.currentText().lower()
        level = receiver.LOG_LEVELS[level_str]
        return level

    def on_filter_change(self, index):
        level = self.get_filter_level()
        self.logger.debug("Filter level changed to: {}".format(level))
        self.apply_filter(level)

    def create_console_widget(self):
        console = QtWidgets.QTreeWidget(self)
        console.setObjectName("Console")
        console.setAllColumnsShowFocus(True)
        console.setHeaderLabels(("", "Source", "Stamp", "Message"))
        header = console.header()
        header.setSectionResizeMode(QtWidgets.QHeaderView.ResizeToContents)
        console.setAlternatingRowColors(True)
        return console

    def add_log_entry(self, log):
        date = datetime.datetime.fromtimestamp(log["timestamp"])

        item = QtWidgets.QTreeWidgetItem(
            ("", str(log["source"]), date.strftime("%d.%m. %H:%M"),
             str(log["message"])))
        level = log["level"]
        style = None
        if level == receiver.LOG_LEVELS["debug"]:
            style = QtWidgets.QStyle.SP_MessageBoxQuestion
            item.severity_level = receiver.LOG_LEVELS["debug"]
            self._item_count["debug"] += 1
        elif level == receiver.LOG_LEVELS["info"]:
            style = QtWidgets.QStyle.SP_MessageBoxInformation
            item.severity_level = receiver.LOG_LEVELS["info"]
            self._item_count["info"] += 1
        elif level == receiver.LOG_LEVELS["warning"]:
            style = QtWidgets.QStyle.SP_MessageBoxWarning
            item.severity_level = receiver.LOG_LEVELS["warning"]
            self._item_count["warning"] += 1
        elif level == receiver.LOG_LEVELS["error"]:
            style = QtWidgets.QStyle.SP_MessageBoxCritical
            item.severity_level = receiver.LOG_LEVELS["error"]
            self._item_count["error"] += 1
        icon = QtGui.QIcon(QtWidgets.QApplication.style().standardIcon(style))
        item.setIcon(0, icon)
        vbar = self.console.verticalScrollBar()
        self.setUpdatesEnabled(False)
        if vbar.value() == vbar.maximum():
            self.console.addTopLevelItem(item)
            self.console.scrollToBottom()
        else:
            self.console.addTopLevelItem(item)
        item_count = self.console.topLevelItemCount()
        self.console.topLevelItem(item_count - 1).setHidden(
            item.severity_level < self.get_filter_level())
        item_count_category = self._item_count[receiver.LOG_LEVELS_REVERSE[
            item.severity_level]]
        if item_count_category > self.LOG_COUNT_LIMIT:
            self._delete_oldest_from_level(item.severity_level)
        self.setUpdatesEnabled(True)
        self.update_status_widget()

    def _delete_oldest_from_level(self, level):
        total_count = self.console.topLevelItemCount()
        for i in range(total_count):
            item = self.console.topLevelItem(i)
            if item.severity_level == level:
                self.logger.debug("Removing item with index {}".format(i))
                self.console.takeTopLevelItem(i)
                self._item_count[receiver.LOG_LEVELS_REVERSE[level]] -= 1
                return True
        return False


class Canvas(FigureCanvas):
    def __init__(self, parent=None, width=5, height=4, dpi=100):
        figure = Figure(figsize=(width, height), dpi=dpi)
        self.axes = figure.add_subplot(111)
        super().__init__(figure)
        self.setParent(parent)

    def set_formatter(self, format):
        x_format = mpldates.DateFormatter(format)
        self.axes.xaxis.set_major_formatter(x_format)


class CustomPlot(QtWidgets.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        layout = QtWidgets.QHBoxLayout()

        canvas = self.create_canvas()
        self._canvas = canvas
        self.settings = {
            "grid": True,
            "x_format": "%H:%M",
            "x_range": 86400,
            "y_autoscale": True,
            "y_range": (-10, 10),
        }

        now = time.time()
        x_data = [
            now - 3600 * 10, now - 3600 * 8.76, now - 3600 * 3.48,
            now - 3600 * 1.32, now
        ]
        y_data = [1, 2, 1.5, 0.1, 5]
        self.new_plot("Time", "EC", "EC-Sensor")
        self.new_line(x_data[:2], y_data[:2], "EC 1")
        self.new_line(x_data[2:], y_data[2:], "EC 2")
        self.remove_line("EC 1")
        layout.addWidget(canvas)
        self.setLayout(layout)

    def create_canvas(self):
        canvas = Canvas(self)
        canvas.setObjectName("Canvas")
        canvas.axes.grid()
        return canvas

    def new_plot(self, x_label, y_label, title):
        canvas = self._get_canvas()
        # clear all lines
        canvas.axes.cla()
        if self.settings["grid"]:
            canvas.axes.grid()
        canvas.axes.set_xlabel(x_label)
        canvas.axes.set_ylabel(y_label)
        canvas.axes.set_title(title)

    def new_line(self, x_data, y_data, name):
        x_data = [datetime.datetime.fromtimestamp(i) for i in x_data]
        x_data = mpldates.date2num(x_data)

        canvas = self._get_canvas()

        canvas.axes.plot(x_data, y_data, label=name)

        # set x-axis limits
        limits = self._get_new_xlim(time.time())
        canvas.axes.set_xlim(limits)

        # set y-axis limits
        self._update_ylim()

        canvas.axes.legend()
        canvas.set_formatter(self.settings["x_format"])
        canvas.draw()

    def remove_line(self, name):
        canvas = self._get_canvas()
        lines = canvas.axes.get_lines()
        for line in lines:
            if line.get_label() == name:
                # delete the line
                canvas.axes.lines.remove(line)
                # update the legend
                canvas.axes.legend()
                return True

        return False

    def append_to_line(self, x_data, y_data, name):
        x_data = [datetime.datetime.fromtimestamp(i) for i in x_data]
        x_data = mpldates.date2num(x_data)

        line = self._get_line(name)
        if not line:
            return

        x_data = np.append(line.get_xdata(), x_data)
        y_data = np.append(line.get_ydata(), y_data)
        line.set_xdata(x_data)
        line.set_ydata(y_data)

        canvas = self._get_canvas()
        limits = self._get_new_xlim(time.time())
        canvas.axes.set_xlim(limits)
        self._update_ylim()
        canvas.draw()

    def get_all_line_names(self):
        canvas = self._get_canvas()
        lines = canvas.axes.get_lines()
        names = []
        for line in lines:
            names.append(line.get_label())
        return names

    def _get_canvas(self):
        return self.findChild(FigureCanvas, "Canvas")

    def _get_line(self, name):
        canvas = self._get_canvas()
        lines = canvas.axes.get_lines()
        for line in lines:
            if line.get_label() == name:
                return line
        return None

    def _get_new_xlim(self, stamp):
        right_date = datetime.datetime.fromtimestamp(stamp)
        right = mpldates.date2num(right_date)
        left = mpldates.date2num(right_date - datetime.timedelta(
            seconds=self.settings["x_range"]))
        return (left, right)

    def _update_ylim(self):
        canvas = self._get_canvas()

        def y_limits():
            lines = canvas.axes.get_lines()
            min_y = np.Inf
            max_y = -np.Inf
            x_min, x_max = canvas.axes.get_xlim()
            for line in lines:
                y_data = line.get_ydata()
                x_data = line.get_xdata()
                y_data_visible = y_data[(x_min <= x_data) & (x_data <= x_max)]
                line_y_min = np.min(y_data_visible)
                line_y_max = np.max(y_data_visible)
                min_y = line_y_min if line_y_min < min_y else min_y
                max_y = line_y_max if line_y_max > max_y else max_y
            return (min_y, max_y)

        if self.settings["y_autoscale"]:
            limits = y_limits()
            margin = np.max(np.abs(limits)) * 0.1

            canvas.axes.set_ylim(limits[0] - margin, limits[1] + margin)
        else:
            canvas.axes.set_ylim(self.settings["y_range"])


class PanelWidget(QtWidgets.QWidget):
    def __init__(self, parent=None, inner_widget=None):
        super(PanelWidget, self).__init__(parent)
        self._border_width = 2
        self._border_radius = 0

        self._title_color = QtGui.QColor(255, 255, 255)
        self._title_font = QtGui.QFont()
        self._title_font.setPointSize(18)
        self._title_text = "Title"
        self._title_height = self.get_title_height()

        self._color = QtGui.QColor(100, 149, 237)

        if inner_widget:
            self.inner_widget = inner_widget
            self.inner_widget.setParent(self)
        else:
            self.inner_widget = QtWidgets.QLCDNumber(self)
            self.inner_widget.setFrameShape(QtWidgets.QFrame.NoFrame)
        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(self.inner_widget)
        self.setLayout(layout)

    def get_title_height(self):
        label = QtWidgets.QLabel(self._title_text)
        label.setFont(self._title_font)
        height = label.fontMetrics().boundingRect(label.text()).height()
        return height

    def paintEvent(self, event):
        painter = QtGui.QPainter(self)
        painter.setRenderHints(QtGui.QPainter.Antialiasing
                               | QtGui.QPainter.TextAntialiasing)

        self.draw_border(painter)
        self.draw_title(painter)

    def draw_border(self, painter: QtGui.QPainter):
        painter.save()
        pen = QtGui.QPen()
        pen.setWidth(self._border_width)
        pen.setColor(self._color)
        painter.setPen(pen)
        painter.setBrush(Qt.NoBrush)
        path = QtGui.QPainterPath()
        rect = QtCore.QRectF(self._border_width / 2, self._border_width / 2,
                             self.width() - self._border_width,
                             self.height() - self._border_width)
        path.addRoundedRect(rect, self._border_radius, self._border_radius)
        painter.drawPath(path)

        painter.restore()

    def draw_title(self, painter: QtGui.QPainter):
        self.layout().setContentsMargins(
            self._border_radius, self._title_height + self._border_radius,
            self._border_radius, self._border_radius)
        painter.save()
        painter.setPen(Qt.NoPen)
        painter.setBrush(self._color)

        offset = int(2 * self._border_width / 3)
        rect = QtCore.QRect(offset, offset,
                            self.width() - offset * 2, self._title_height)
        painter.drawRect(rect)

        painter.setPen(self._title_color)
        painter.setFont(self._title_font)
        offset = self._border_width * 3
        text_rect = QtCore.QRect(offset, 0,
                                 self.width() - offset * 2, self._title_height)
        align = Qt.Alignment(Qt.AlignHCenter | Qt.AlignVCenter)
        painter.drawText(text_rect, align, self._title_text)
        painter.restore()

    @QtCore.pyqtSlot(str)
    def set_title(self, title: str):
        self._title_text = title

    @QtCore.pyqtSlot(QObject)
    def set_inner_widget(self, widget: QtWidgets.QWidget):
        self.layout().removeWidget(self.inner_widget)
        self.inner_widget.setParent(None)
        self.inner_widget = widget
        self.layout().addWidget(self.inner_widget)


class LcdPanelWidget(PanelWidget):
    def __init__(self, parent=None, title="LCD"):
        super(LcdPanelWidget,
              self).__init__(parent=parent,
                             inner_widget=QtWidgets.QLCDNumber())
        self.inner_widget.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.set_title(title)
        self._digit_count = 7

    @QtCore.pyqtSlot(int)
    def set_number_int(self, number: int):
        disp_string = "{}".format(number).center(self._digit_count)
        self.inner_widget.setDigitCount(self._digit_count)
        self.inner_widget.display(disp_string)

    @QtCore.pyqtSlot(float)
    def set_number_float(self, number: float):
        disp_string = "{:.2f}".format(number).center(self._digit_count)
        self.inner_widget.setDigitCount(self._digit_count)
        self.inner_widget.display(disp_string)

    @QtCore.pyqtSlot()
    def set_number_none(self):
        disp_string = "-".center(self._digit_count)
        self.inner_widget.setDigitCount(self._digit_count)
        self.inner_widget.display(disp_string)

class MinMaxAvgLcdWidget(QtWidgets.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent=parent)
        self._digit_count = 7
        layout = QtWidgets.QVBoxLayout()
        min_palette = QtGui.QPalette()
        min_palette.setColor(QtGui.QPalette.WindowText, QtGui.QColor(100, 149, 237))
        max_palette = QtGui.QPalette()
        max_palette.setColor(QtGui.QPalette.WindowText, QtGui.QColor(255, 50, 50))
        self.min_lcd = QtWidgets.QLCDNumber(self)
        self.min_lcd.setPalette(min_palette)
        self.min_lcd.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.max_lcd = QtWidgets.QLCDNumber(self)
        self.max_lcd.setPalette(max_palette)
        self.max_lcd.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.avg_lcd = QtWidgets.QLCDNumber(self)
        self.avg_lcd.setFrameShape(QtWidgets.QFrame.NoFrame)
        layout.addWidget(self.max_lcd)
        layout.addWidget(self.avg_lcd)
        layout.addWidget(self.min_lcd)
        self.setLayout(layout)

    def _set_float(self, widget, number):
        disp_string = "{:.2f}".format(number).center(self._digit_count)
        widget.setDigitCount(self._digit_count)
        widget.display(disp_string)

    def _set_none(self, widget):
        disp_string = "-".center(self._digit_count)
        widget.setDigitCount(self._digit_count)
        widget.display(disp_string)
    
    def set_max_float(self, number):
        self._set_float(self.max_lcd, number)

    def set_max_none(self):
        self._set_none(self.max_lcd)

    def set_min_float(self, number):
        self._set_float(self.min_lcd, number)

    def set_min_none(self):
        self._set_none(self.min_lcd)

    def set_avg_float(self, number):
        self._set_float(self.avg_lcd, number)

    def set_avg_none(self):
        self._set_none(self.avg_lcd)

    



class MinMaxAvgLcdPanelWidget(PanelWidget):
    def __init__(self, parent=None, title="LCD"):
        super().__init__(parent=parent, inner_widget=MinMaxAvgLcdWidget())
        self.set_title = title
    
    def set_min_max_avg(self, min_val, max_val, avg_val):
        self.inner_widget.set_min_float(float(min_val))
        self.inner_widget.set_max_float(float(max_val))
        self.inner_widget.set_avg_float(float(avg_val))


class LcdGridWidget(QtWidgets.QWidget):
    def __init__(self, size, titles, parent=None):
        super(LcdGridWidget, self).__init__(parent)
        lcd_count = size[0] * size[1]
        while len(titles) < lcd_count:
            titles.append("Unnamed Title")
        layout = QtWidgets.QGridLayout()
        self.widgets = []
        self.grid_size = size

        for row in range(size[0]):
            for col in range(size[1]):
                index = row * size[1] + col
                widget = LcdPanelWidget(self, titles[index])
                widget.set_number_none()
                self.widgets.append(widget)
                layout.addWidget(widget, row, col)

        self.setLayout(layout)

    @QtCore.pyqtSlot(int, int)
    def set_number_int(self, index, value):
        self.widgets[index].set_number_int(value)

    @QtCore.pyqtSlot(int, float)
    def set_number_float(self, index, value):
        self.widgets[index].set_number_float(value)

    @QtCore.pyqtSlot(int)
    def set_number_none(self, index):
        self.widgets[index].set_number_none()

    def replace_widget(self, index, widget):
        self.layout().removeWidget(self.widgets[index])
        self.widgets[index].close()
        self.widgets[index] = widget
        row = index // self.grid_size[1]
        col = index % self.grid_size[1]
        self.layout().addWidget(self.widgets[index], row, col)


class OverviewWidget(LcdGridWidget):
    def __init__(self, parent=None):
        titles = [
            "EC [µS/cm]", "pH", "Water Temperature [°C]",
            "LED Temperature [°C]", "Air Temperature [°C]", "Humidity [%]"
        ]
        self.index = dict(
            ec=0,
            ph=1,
            water=2,
            led=3,
            air=4,
            humidity=5,
        )
        size = (2, 3)
        super(OverviewWidget, self).__init__(size=size, titles=titles, parent=parent)
        index = self.index["led"]
        widget = MinMaxAvgLcdPanelWidget(parent=self, title=titles[index])
        self.replace_widget(index, widget)

    def set_ec_value(self, value):
        index = self.index["ec"]
        self.set_number_int(index, int(value))

    def clear_ec_value(self):
        index = self.index["ec"]
        self.set_number_none(index)

    
    def set_ph_value(self, value):
        index = self.index["ph"]
        self.set_number_float(index, float(value))

    def clear_ph_value(self):
        index = self.index["ph"]
        self.set_number_none(index)
    
    def set_water_temperature(self, value):
        index = self.index["water"]
        self.set_number_float(index, float(value))

    def clear_water_temperature(self):
        index = self.index["water"]
        self.set_number_none(index)
    
    def set_led_temperature(self, min_val, max_val, avg_val):
        index = self.index["led"]
        self.widgets[index].set_min_max_avg(min_val, max_val, avg_val)

    def clear_led_temeprature(self):
        index = self.index["led"]
        self.set_number_none(index)
    
    def set_air_temperature(self, value):
        index = self.index["air"]
        self.set_number_float(index, float(value))

    def clear_air_temperature(self):
        index = self.index["air"]
        self.set_number_none(index)

    def set_humidity(self, value):
        index = self.index["humidity"]
        self.set_number_int(index, int(value))

    def clear_humidity(self):
        index = self.index["humidity"]
        self.set_number_none(index)
    



if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    win = MainWindow()
    app.exec_()