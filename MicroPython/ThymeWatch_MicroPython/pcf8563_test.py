import machine
from pcf8563 import PCF8563

i2c = machine.I2C(scl=machine.Pin(20), sda=machine.Pin(21))
rtc = PCF8563(i2c)

print(rtc.datetime())