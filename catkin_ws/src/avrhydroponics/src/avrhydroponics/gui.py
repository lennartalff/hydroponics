# -*- coding: utf-8 -*-
from PyQt4.QtCore import Qt, QObject, pyqtSignal
from PyQt4 import QtCore
from PyQt4 import QtGui

class MainWindow(QtGui.QWidget):
    def __init__(self, parent=None):
        super(MainWindow, self).__init__(parent)
        self.setWindowTitle("Test")

        layout = QtGui.QVBoxLayout()
        self.tab_widget = QtGui.QTabWidget(self)
        layout.addWidget(self.tab_widget)

        widget = OverviewWidget(parent=self)
        widget.setObjectName("overview")
        self.tab_widget.addTab(widget, "Overview")

        self.setLayout(layout)
        self.showMaximized()

    def get_overview_widget(self):
        return self.tab_widget.findChild(QtGui.QWidget, "overview")

class PanelWidget(QtGui.QWidget):
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
            self.inner_widget = QtGui.QLCDNumber(self)
            self.inner_widget.setFrameShape(QtGui.QFrame.NoFrame)
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.inner_widget)
        self.setLayout(layout)

    def get_title_height(self):
        label = QtGui.QLabel(self._title_text)
        label.setFont(self._title_font)
        height = label.fontMetrics().boundingRect(label.text()).height()
        return height

    def paintEvent(self, event):
        painter = QtGui.QPainter(self)
        painter.setRenderHints(QtGui.QPainter.Antialiasing
                               | QtGui.QPainter.TextAntialiasing)

        self.draw_border(painter)
        self.draw_title(painter)

    def draw_border(self, painter):
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

    def draw_title(self, painter):
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
    def set_title(self, title):
        self._title_text = title

    @QtCore.pyqtSlot(QObject)
    def set_inner_widget(self, widget):
        self.layout().removeWidget(self.inner_widget)
        self.inner_widget.setParent(None)
        self.inner_widget = widget
        self.layout().addWidget(self.inner_widget)

class LcdPanelWidget(PanelWidget):
    def __init__(self, parent=None, title="LCD"):
        super(LcdPanelWidget,
              self).__init__(parent=parent,
                             inner_widget=QtGui.QLCDNumber())
        self.inner_widget.setFrameShape(QtGui.QFrame.NoFrame)
        self.set_title(title)
        self._digit_count = 7

    @QtCore.pyqtSlot(int)
    def set_number_int(self, number):
        disp_string = "{}".format(number).center(self._digit_count)
        self.inner_widget.setDigitCount(self._digit_count)
        self.inner_widget.display(disp_string)

    @QtCore.pyqtSlot(float)
    def set_number_float(self, number):
        disp_string = "{:.2f}".format(number).center(self._digit_count)
        self.inner_widget.setDigitCount(self._digit_count)
        self.inner_widget.display(disp_string)

    @QtCore.pyqtSlot()
    def set_number_none(self):
        disp_string = "-".center(self._digit_count)
        self.inner_widget.setDigitCount(self._digit_count)
        self.inner_widget.display(disp_string)

class MultiValueLcdWidget(QtGui.QWidget):
    def __init__(self, parent=None, n=2):
        super(MultiValueLcdWidget, self).__init__(parent=parent)
        layout = QtGui.QVBoxLayout()
        self.lcds = []
        for i in range(n):
            self.lcds.append(QtGui.QLCDNumber(self))
            self.lcds[i].setFrameShape(QtGui.QFrame.NoFrame)
            layout.addWidget(self.lcds[i])
        self.setLayout(layout)

class MultiValueLcdPanelWidget(PanelWidget):
    def __init__(self, parent=None, title="MultiValueLCD"):
        super(MultiValueLcdPanelWidget, self).__init__(parent=parent, inner_widget=MultiValueLcdWidget())
        self.set_title(title)
        self._digit_count = 7

    def set_number_int(self, index, number):
        disp_string = "{}".format(number).center(self._digit_count)
        self.inner_widget.lcds[index].setDigitCount(self._digit_count)
        self.inner_widget.lcds[index].display(disp_string)

    def set_number_float(self, index, number):
        disp_string = "{:.2f}".format(number).center(self._digit_count)
        self.inner_widget.lcds[index].setDigitCount(self._digit_count)
        self.inner_widget.lcds[index].display(disp_string)


class MinMaxAvgLcdWidget(QtGui.QWidget):
    def __init__(self, parent=None):
        super(MinMaxAvgLcdWidget, self).__init__(parent=parent)
        self._digit_count = 7
        layout = QtGui.QVBoxLayout()
        min_palette = QtGui.QPalette()
        min_palette.setColor(QtGui.QPalette.WindowText, QtGui.QColor(100, 149, 237))
        max_palette = QtGui.QPalette()
        max_palette.setColor(QtGui.QPalette.WindowText, QtGui.QColor(255, 50, 50))
        self.min_lcd = QtGui.QLCDNumber(self)
        self.min_lcd.setPalette(min_palette)
        self.min_lcd.setFrameShape(QtGui.QFrame.NoFrame)
        self.max_lcd = QtGui.QLCDNumber(self)
        self.max_lcd.setPalette(max_palette)
        self.max_lcd.setFrameShape(QtGui.QFrame.NoFrame)
        self.avg_lcd = QtGui.QLCDNumber(self)
        self.avg_lcd.setFrameShape(QtGui.QFrame.NoFrame)
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
        super(MinMaxAvgLcdPanelWidget, self).__init__(parent=parent, inner_widget=MinMaxAvgLcdWidget())
        self.set_title(title)
    
    def set_min_max_avg(self, min_val, max_val, avg_val):
        self.inner_widget.set_min_float(float(min_val))
        self.inner_widget.set_max_float(float(max_val))
        self.inner_widget.set_avg_float(float(avg_val))


class LcdGridWidget(QtGui.QWidget):
    def __init__(self, size, titles, parent=None):
        super(LcdGridWidget, self).__init__(parent)
        lcd_count = size[0] * size[1]
        while len(titles) < lcd_count:
            titles.append("Unnamed Title")
        layout = QtGui.QGridLayout()
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
        self.setObjectName("overview")
        index = self.index["led"]
        widget = MinMaxAvgLcdPanelWidget(parent=self, title=titles[index])
        self.replace_widget(index, widget)
        index = self.index["ec"]
        widget = MultiValueLcdPanelWidget(parent=self, title=titles[index])
        self.replace_widget(index, widget)

    def set_ec_raw_value(self, value):
        index = self.index["ec"]
        self.widgets[index].set_number_int(1, int(value))
    
    def set_ec_compensated_value(self, value):
        index = self.index["ec"]
        self.widgets[index].set_number_int(0, int(value))
    
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