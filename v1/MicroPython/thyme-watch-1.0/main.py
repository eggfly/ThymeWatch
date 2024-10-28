import gc
from sharp_mem_lcd import SharpMemLCD
import time
import machine
import framebuf
from machine import I2C,Pin,ADC
from drivers import *
from bmi270 import BMI270


i2c = I2C(scl=Pin(20), sda=Pin(21))
print(i2c.scan())
r = PCF8563(i2c)
sht = SHT30()
# bmp = BMP280(i2c)
# bmp.use_case(BMP280_CASE_WEATHER)
# bmp.oversample(BMP280_OS_HIGH)
# imu = BMI270(i2c)
button_m=Pin(9,Pin.IN,Pin.PULL_UP)

lcd = SharpMemLCD()
lcd.fill(1)
lcd.font_load("GB2312-16.fon")
lcd.font_set(0x12,0,1,0)
#字体(第一位1-4对应标准，方头，不等宽标准，不等宽方头，第二位1-4对应12，16，24，32高度)，旋转，放大倍数，反白显示
lcd.text("你唤醒了WATCH",0,0,0)
temperature, humidity = sht.measure()
lcd.text("温度%.4f"%(temperature),0,16,0)
lcd.text("湿度%.4f"%(humidity),0,32,0)
# pre=bmp.pressure
# lcd.text("气压%.4f"%(pre),0,48,0)
# lcd.text('{:>3.2f}{:>3.2f}{:>3.2f}'.format(*imu.accel()),0,64,0)
# lcd.text('{:>3.2f}{:>3.2f}{:>3.2f}'.format(*imu.gyro()),0,80,0)
lcd.text("按中键继续睡去",0,110,0)
lcd.show()

while 1:
    if not button_m.value():
        lcd.text("继续睡",0,90,0)
        lcd.show()
        time.sleep_ms(500)
        hh=str(r.hours())
        mm=str(r.minutes())
        if len(hh)==1:
            hh='0'+hh
        if len(mm)==1:
            mm='0'+mm
        s1=int(hh[0])
        s2=int(hh[1])
        s3=int(mm[0])
        s4=int(mm[1])
        pot = ADC(Pin(2))
        pot.atten(ADC.ATTN_11DB)
        vbat = pot.read() * 6.6 / 4095.0 
        lcd.fill(1)
        lcd.font_set(0x22,0,1,0)
        boot_ms = time.ticks_ms()
        lcd.text("%.3fV %dms" %(vbat, boot_ms), 0,5,0)
        lcd.font_set(0x34,0,2,0)
        lcd.text(hh,30,10,0)
        lcd.text(mm,30,63,0)
        lcd.show()
        time.sleep_ms(500)
        machine.deepsleep(59000)
        
                
        




