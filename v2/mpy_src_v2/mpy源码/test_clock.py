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
from machine import SPI,Pin
from drivers import LPM013M126A
import drivers
import time
from machine import SoftI2C
import machine

i2c=SoftI2C(scl=I2C_SCL,sda=I2C_SDA)
rtc = drivers.RTC(i2c)
rtc.DS3231_SetTime(b"\x00\x53\x15\x04\x29\x10\x24")



