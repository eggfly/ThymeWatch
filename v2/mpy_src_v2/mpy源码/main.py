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

from machine import SPI,Pin,SoftI2C
from drivers import LPM013M126A
import drivers
import time
from contra import *
import ufont
import machine

spi = SPI(baudrate=20000000,polarity=0, phase=0,firstbit=SPI.MSB,sck=Pin(LCD_SCK),mosi=Pin(LCD_SI ),miso=Pin(0))
lcd=LPM013M126A(spi,LCD_CS,LCD_BL)
#lcd.set_bl(1000)

i2c=SoftI2C(scl=I2C_SCL,sda=I2C_SDA)
rtc = drivers.RTC(i2c)
t = rtc.DS3231_ReadTime(0)

lcd.blit(contra_fb,0,0)
print(t)
font = ufont.BMFont("mario.bmf")
font.text(lcd, str(t[2])+":"+str(t[1]), 38, 40, color_type=1,font_size=40, color=12, bg_color=0, show=True, clear=True)
font.text(lcd, str(t[3])+" "+str(t[5])+"-"+str(t[4]), 10, 140, color_type=1,font_size=32, color=14, bg_color=0, show=True, clear=True)

lcd.show()

import esp32
print('wake up')
wake1 = Pin(4, mode = Pin.IN)
wake2 = Pin(0, mode = Pin.IN)
esp32.wake_on_ext1(pins = (wake1,wake2), level = esp32.WAKEUP_ALL_LOW)

if wake1.value()==1:   #按住中键不松手就不会睡去
    machine.deepsleep(59500)   #每分钟一刷







