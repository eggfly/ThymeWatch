#-------------------------------------------------SETTINGS---------------------------------------------
#PIN
TP_INT = const(0)
IMU_INT = const(1)
RTC_INT = const(2)
BUTTON_UP = const(4)
LCD_BL = const(5)
LCD_SCK = const(6)
LCD_SI = const(7)
LCD_CS = const(8)
BUTTON_DOWN = const(9)
TP_RST = const(10)
I2C_SCL = const(20)
I2C_SDA = const(21)
#I2C
from machine import SoftI2C
i2c=SoftI2C(scl=I2C_SCL,sda=I2C_SDA)
#print(i2c.scan())

#-------------------------------------------------LPM013M126A----------------------------------------------
# MicroPython driver for the JDI LPM013M126A 176x176 Color Memory LCD
# Author: [jd3096]

import machine
import time
import framebuf
from machine import SPI,Pin,PWM

#COLOR
LCD_COLOR_BLACK     = const(0x00)   
LCD_COLOR_BLUE      = const(0x02)   
LCD_COLOR_GREEN     = const(0x04)   
LCD_COLOR_CYAN      = const(0x06)   
LCD_COLOR_RED       = const(0x08)    
LCD_COLOR_MAGENTA   = const(0x0a)    
LCD_COLOR_YELLOW    = const(0x0c)    
LCD_COLOR_WHITE     = const(0x0e)    

# Commands for the LCD
LCD_COLOR_CMD_UPDATE_1BIT = 0x88
LCD_COLOR_CMD_UPDATE_3BIT = 0x80
LCD_COLOR_CMD_UPDATE_4BIT = 0x90
LCD_COLOR_CMD_ALL_CLEAR = 0x20

LCD_COLOR_CMD_NO_UPDATE = 0x00
LCD_COLOR_CMD_BLINKING_WHITE = 0x18
LCD_COLOR_CMD_BLINKING_BLACK = 0x10
LCD_COLOR_CMD_INVERSION = 0x14

LCD_WIDTH = 176
LCD_HEIGHT = 176

class LPM013M126A(framebuf.FrameBuffer):
    def __init__(self, spi, cs, bl,width=LCD_WIDTH, height=LCD_HEIGHT):
        self.bl=PWM(Pin(bl), freq=1000)
        self.bl.duty(0)
        self.spi = spi
        self.cs = Pin(cs,Pin.OUT)
        self.cs.value(0)
        self.width = width
        self.height = height
        self.polarity = 0
        self.blkcmd = 0x00
        self.colorcmd = 0x90
        self.buffer = bytearray((self.width * self.height) // 2)  # 4 bits per pixel
        super().__init__(self.buffer, self.width, self.height, framebuf.GS4_HMSB)
        # Clear the display buffer
        self.set_blink_mode('none')
        self.clear()
    
    def set_bl(self,lux):
        #Between 0-1023
        if lux>1023:
            lux=1023
        if lux<0:
            lux=0
        self.bl.duty(lux)
        
    def toggle_vcom(self):
        pass
        #self.polarity ^= 0x40  # Toggle bit 6
        
    def clear(self):
        self.send_command(LCD_COLOR_CMD_ALL_CLEAR)
        
    def send_command(self, command):
        self.cs.value(1) 
        time.sleep_us(6)
        self.spi.write((command | self.polarity).to_bytes(1,'big'))
        self.toggle_vcom()
        self.spi.write(b'\x00')
        time.sleep_us(6)
        self.cs.value(0) 
        
    def show(self):
        # Send the buffer to the display
        self.cs.value(1) 
        time.sleep_us(6)
        self.spi.write((self.colorcmd | self.polarity).to_bytes(1,'big'))
        self.toggle_vcom()
        for y in range(self.height):
            line_address = y + 1
            self.spi.write(line_address.to_bytes(1,'big'))
            line_start = (self.width // 2) * y
            line_end = line_start + (self.width // 2)
            self.spi.write(self.buffer[line_start:line_end])
            self.spi.write(b'\x00')  # Dummy byte at the end of each line
        self.spi.write(b'\x00')  # Final dummy byte
        time.sleep_us(6)
        self.cs.value(0)  
    
    def show_lines(self,start,end,fb):
        self.cs.value(1) 
        time.sleep_us(6)
        self.spi.write((self.colorcmd | self.polarity).to_bytes(1,'big'))
        self.toggle_vcom()
        for y in range(start,end):
            line_address = start + 1
            self.spi.write(line_address.to_bytes(1,'big'))
            line_start = (self.width // 2) * y
            line_end = line_start + (self.width // 2)
            self.spi.write(self.buffer[line_start:line_end])
            self.spi.write(b'\x00')  # Dummy byte at the end of each line
        self.spi.write(b'\x00')  # Final dummy byte
        time.sleep_us(6)
        self.cs.value(0)
        
    def set_blink_mode(self, mode):
        if mode == 'none':
            self.blkcmd = LCD_COLOR_CMD_NO_UPDATE
        elif mode == 'white':
            self.blkcmd = LCD_COLOR_CMD_BLINKING_WHITE
        elif mode == 'black':
            self.blkcmd = LCD_COLOR_CMD_BLINKING_BLACK
        elif mode == 'inverse':
            self.blkcmd = LCD_COLOR_CMD_INVERSION
        else:
            self.blkcmd = LCD_COLOR_CMD_NO_UPDATE
        self.send_command(self.blkcmd)
        
    def set_color_mode(self, mode):
        if mode == 1:
            self.colorcmd = LCD_COLOR_CMD_UPDATE_1BIT
        elif mode == 3:
            self.colorcmd = LCD_COLOR_CMD_UPDATE_3BIT
        elif mode == 4:
            self.colorcmd = LCD_COLOR_CMD_UPDATE_4BIT
        else:
            self.self.colorcmd = LCD_COLOR_CMD_UPDATE_4BIT
            

#-------------------------------------------------DS3231----------------------------------------------
import machine

class RTC:
    w = ["FRI", "SAT", "SUN", "MON", "TUE", "WED", "THU"]
    # If you want different names for Weekdays, feel free to add. Couple examples below:
    # w = ["FR", "SA", "SU", "MO", "TU", "WE", "TH"]
    # w = ["Friday", "Saturday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday"]

    def __init__(self, i2c, port=0, speed=100000, address=0x68, register=0x00):
        self.rtc_address = address  
        self.rtc_register = register
        self.i2c = i2c

    def DS3231_SetTime(self, NowTime=b"\x00\x23\x12\x28\x14\x07\x21"):
        # NowTime has to be in format like b'\x00\x23\x12\x28\x14\x07\x21'
        # It is encoded like this           sec min hour week day month year
        # Then it's written to the DS3231
        self.i2c.writeto_mem(int(self.rtc_address), int(self.rtc_register), NowTime)

    # DS3231 gives data in bcd format. This has to be converted to a binary format.
    def bcd2bin(self, value):
        return (value or 0) - 6 * ((value or 0) >> 4)

    # Add a 0 in front of numbers smaller than 10
    def pre_zero(self, value):
        pre_zero = True  # Change to False if you don't want a "0" in front of numbers smaller than 10
        if pre_zero:
            if value < 10:
                value = f"0{value}"  # From now on the value is a string!
        return value

    def DS3231_ReadTime(self, mode=0):
        try:
            # Read RT from DS3231 and write to the buffer variable. It's a list with 7 entries.
            # Every entry needs to be converted from bcd to bin.
            buffer = self.i2c.readfrom_mem(self.rtc_address, self.rtc_register, 7)
            # The year consists of 2 digits. Here 2000 years are added to get format like "2021"
            year = self.bcd2bin(buffer[6]) + 2000
            month = self.bcd2bin(buffer[5])  # Just put the month value in the month variable and convert it.
            day = self.bcd2bin(buffer[4])  # Same for the day value
            # Weekday will be converted in the weekdays name or shortform like "Sunday" or "SUN"
            weekday = self.w[self.bcd2bin(buffer[3])]
            # Uncomment the line below if you want a number for the weekday and comment the line before.
            # weekday = self.bcd2bin(buffer[3])
            hour = self.pre_zero(self.bcd2bin(buffer[2]))  # Convert bcd to bin and add a "0" if necessary
            minute = self.pre_zero(self.bcd2bin(buffer[1]))  # Convert bcd to bin and add a "0" if necessary
            second = self.pre_zero(self.bcd2bin(buffer[0]))  # Convert bcd to bin and add a "0" if necessary
            if mode == 0:  # Mode 0 returns a list of second, minute, ...
                return second, minute, hour, weekday, day, month, year
            if mode == 1:  # Mode 1 returns a formated string with time, weekday and date
                time_string = f"{hour}:{minute}:{second}      {weekday} {day}.{month}.{year}"
                return time_string
            # If you need different format, feel free to add

        except Exception as e:
            return (
                "Error: is the DS3231 not connected or some other problem (%s)" % e
            )  # exception occurs in any case of error.
        
        
#-------------------------------------------------INA219----------------------------------------------
"""
`adafruit_ina219`
====================================================

CircuitPython driver for the INA219 current sensor.

* Author(s): Dean Miller
"""

from micropython import const

__repo__ = "https://github.com/adafruit/Adafruit_CircuitPython_INA219.git"

# Bits
# pylint: disable=bad-whitespace
_READ = const(0x01)

# Config Register (R/W)
_REG_CONFIG = const(0x00)
_CONFIG_RESET = const(0x8000)  # Reset Bit

_CONFIG_BVOLTAGERANGE_MASK = const(0x2000)  # Bus Voltage Range Mask
_CONFIG_BVOLTAGERANGE_16V = const(0x0000)  # 0-16V Range
_CONFIG_BVOLTAGERANGE_32V = const(0x2000)  # 0-32V Range

_CONFIG_GAIN_MASK = const(0x1800)     # Gain Mask
_CONFIG_GAIN_1_40MV = const(0x0000)   # Gain 1, 40mV Range
_CONFIG_GAIN_2_80MV = const(0x0800)   # Gain 2, 80mV Range
_CONFIG_GAIN_4_160MV = const(0x1000)  # Gain 4, 160mV Range
_CONFIG_GAIN_8_320MV = const(0x1800)  # Gain 8, 320mV Range

_CONFIG_BADCRES_MASK = const(0x0780)   # Bus ADC Resolution Mask
_CONFIG_BADCRES_9BIT = const(0x0080)   # 9-bit bus res = 0..511
_CONFIG_BADCRES_10BIT = const(0x0100)  # 10-bit bus res = 0..1023
_CONFIG_BADCRES_11BIT = const(0x0200)  # 11-bit bus res = 0..2047
_CONFIG_BADCRES_12BIT = const(0x0400)  # 12-bit bus res = 0..4097

_CONFIG_SADCRES_MASK = const(0x0078)              # Shunt ADC Res. &  Avg. Mask
_CONFIG_SADCRES_9BIT_1S_84US = const(0x0000)      # 1 x 9-bit shunt sample
_CONFIG_SADCRES_10BIT_1S_148US = const(0x0008)    # 1 x 10-bit shunt sample
_CONFIG_SADCRES_11BIT_1S_276US = const(0x0010)    # 1 x 11-bit shunt sample
_CONFIG_SADCRES_12BIT_1S_532US = const(0x0018)    # 1 x 12-bit shunt sample
_CONFIG_SADCRES_12BIT_2S_1060US = const(0x0048)   # 2 x 12-bit sample average
_CONFIG_SADCRES_12BIT_4S_2130US = const(0x0050)   # 4 x 12-bit sample average
_CONFIG_SADCRES_12BIT_8S_4260US = const(0x0058)   # 8 x 12-bit sample average
_CONFIG_SADCRES_12BIT_16S_8510US = const(0x0060)  # 16 x 12-bit sample average
_CONFIG_SADCRES_12BIT_32S_17MS = const(0x0068)    # 32 x 12-bit sample average
_CONFIG_SADCRES_12BIT_64S_34MS = const(0x0070)    # 64 x 12-bit sample average
_CONFIG_SADCRES_12BIT_128S_69MS = const(0x0078)   # 128 x 12-bit sample average

_CONFIG_MODE_MASK = const(0x0007)  # Operating Mode Mask
_CONFIG_MODE_POWERDOWN = const(0x0000)
_CONFIG_MODE_SVOLT_TRIGGERED = const(0x0001)
_CONFIG_MODE_BVOLT_TRIGGERED = const(0x0002)
_CONFIG_MODE_SANDBVOLT_TRIGGERED = const(0x0003)
_CONFIG_MODE_ADCOFF = const(0x0004)
_CONFIG_MODE_SVOLT_CONTINUOUS = const(0x0005)
_CONFIG_MODE_BVOLT_CONTINUOUS = const(0x0006)
_CONFIG_MODE_SANDBVOLT_CONTINUOUS = const(0x0007)

# SHUNT VOLTAGE REGISTER (R)
_REG_SHUNTVOLTAGE = const(0x01)

# BUS VOLTAGE REGISTER (R)
_REG_BUSVOLTAGE = const(0x02)

# POWER REGISTER (R)
_REG_POWER = const(0x03)

# CURRENT REGISTER (R)
_REG_CURRENT = const(0x04)

# CALIBRATION REGISTER (R/W)
_REG_CALIBRATION = const(0x05)
# pylint: enable=bad-whitespace


def _to_signed(num):
    if num > 0x7FFF:
        num -= 0x10000
    return num


class INA219:
    """Driver for the INA219 current sensor"""
    def __init__(self, i2c_device, addr=0x40):
        self.i2c_device = i2c_device

        self.i2c_addr = addr
        self.buf = bytearray(2)
        # Multiplier in mA used to determine current from raw reading
        self._current_lsb = 0
        # Multiplier in W used to determine power from raw reading
        self._power_lsb = 0

        # Set chip to known config values to start
        self._cal_value = 4096
        self.set_calibration_32V_2A()

    def _write_register(self, reg, value):
        self.buf[0] = (value >> 8) & 0xFF
        self.buf[1] = value & 0xFF
        self.i2c_device.writeto_mem(self.i2c_addr, reg, self.buf)

    def _read_register(self, reg):
        self.i2c_device.readfrom_mem_into(self.i2c_addr, reg & 0xff, self.buf)
        value = (self.buf[0] << 8) | (self.buf[1])
        return value

    @property
    def shunt_voltage(self):
        """The shunt voltage (between V+ and V-) in Volts (so +-.327V)"""
        value = _to_signed(self._read_register(_REG_SHUNTVOLTAGE))
        # The least signficant bit is 10uV which is 0.00001 volts
        return value * 0.00001

    @property
    def bus_voltage(self):
        """The bus voltage (between V- and GND) in Volts"""
        raw_voltage = self._read_register(_REG_BUSVOLTAGE)

        # Shift to the right 3 to drop CNVR and OVF and multiply by LSB
        # Each least signficant bit is 4mV
        voltage_mv = _to_signed(raw_voltage >> 3) * 4
        return voltage_mv * 0.001

    @property
    def current(self):
        """The current through the shunt resistor in milliamps."""
        # Sometimes a sharp load will reset the INA219, which will
        # reset the cal register, meaning CURRENT and POWER will
        # not be available ... athis by always setting a cal
        # value even if it's an unfortunate extra step
        self._write_register(_REG_CALIBRATION, self._cal_value)

        # Now we can safely read the CURRENT register!
        raw_current = _to_signed(self._read_register(_REG_CURRENT))
        return raw_current * self._current_lsb

    def set_calibration_32V_2A(self):  # pylint: disable=invalid-name
        self._current_lsb = .1  # Current LSB = 100uA per bit
        self._cal_value = 4096
        self._power_lsb = .002  # Power LSB = 2mW per bit
        self._write_register(_REG_CALIBRATION, self._cal_value)

        # Set Config register to take into account the settings above
        config = (_CONFIG_BVOLTAGERANGE_32V |
                  _CONFIG_GAIN_8_320MV |
                  _CONFIG_BADCRES_12BIT |
                  _CONFIG_SADCRES_12BIT_1S_532US |
                  _CONFIG_MODE_SANDBVOLT_CONTINUOUS)
        self._write_register(_REG_CONFIG, config)

    def set_calibration_32V_1A(self):  # pylint: disable=invalid-name
        """Configures to INA219 to be able to measure up to 32V and 1A of
           current. Counter overflow occurs at 1.3A.

           .. note:: These calculations assume a 0.1 ohm shunt resistor."""
        self._current_lsb = 0.04  # In milliamps
        self._cal_value = 10240
        self._power_lsb = 0.0008
        self._write_register(_REG_CALIBRATION, self._cal_value)

        # Set Config register to take into account the settings above
        config = (_CONFIG_BVOLTAGERANGE_32V |
                  _CONFIG_GAIN_8_320MV |
                  _CONFIG_BADCRES_12BIT |
                  _CONFIG_SADCRES_12BIT_1S_532US |
                  _CONFIG_MODE_SANDBVOLT_CONTINUOUS)
        self._write_register(_REG_CONFIG, config)

    def set_calibration_16V_400mA(self):  # pylint: disable=invalid-name
        """Configures to INA219 to be able to measure up to 16V and 400mA of
           current. Counter overflow occurs at 1.6A.

           .. note:: These calculations assume a 0.1 ohm shunt resistor."""
        self._current_lsb = 0.05  # in milliamps
        self._cal_value = 8192
        self._power_lsb = 0.001

        # Set Calibration register to 'Cal' calculated above
        self._write_register(_REG_CALIBRATION, self._cal_value)

        # Set Config register to take into account the settings above
        config = (_CONFIG_BVOLTAGERANGE_16V |
                  _CONFIG_GAIN_1_40MV |
                  _CONFIG_BADCRES_12BIT |
                  _CONFIG_SADCRES_12BIT_1S_532US |
                  _CONFIG_MODE_SANDBVOLT_CONTINUOUS)
        self._write_register(_REG_CONFIG, config)
        
#-------------------------------------------------EXAMPLES----------------------------------------------
# #SCREEN
# spi = SPI(baudrate=20000000,polarity=0, phase=0,firstbit=SPI.MSB,sck=Pin(6),mosi=Pin(7),miso=Pin(0))
# lcd=LPM013M126A(spi,8,5)
# import ufont
# font = ufont.BMFont("mario.bmf")
# font.text(lcd, "21:48", 45, 35, color_type=1,font_size=32, color=9, bg_color=6, show=True, clear=True)
# lcd.show()
        
# #DS3231
# #https://github.com/balance19/micropython_DS3231/blob/main/my_lib/RTC_DS3231.py
# import ds3231
# import time
# 
# rtc = ds3231.RTC(i2c)
# # It is encoded like this: sec min hour week day month year.
# #rtc.DS3231_SetTime(b"\x00\x45\x15\x03\x28\x10\x24")
# while True:
#     t = rtc.DS3231_ReadTime(1)  # Read RTC and receive data in Mode 1 (see /my_lib/RTC_DS3231.py)
#     print(t)
#     time.sleep(1)
# #INA219
# #https://github.com/robert-hh/INA219
# from machine import I2C
# from ina219 import INA219
# import time
# 
# ina = INA219(i2c)
# 
# ina.set_calibration_32V_1A()
# 
# while True:
#     current = ina.current
#     voltage = ina.bus_voltage
#     print("{} mA  {} V".format(current, voltage))
#     time.sleep(1)