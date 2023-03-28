from sharp_mem_lcd import SharpMemLCD
import time,esp32
import machine
from machine import I2C,Pin,ADC
from drivers import PCF8563

#WAKE-UP BUTTON
wake1 = machine.Pin(4, machine.Pin.IN, machine.Pin.PULL_DOWN)
wake2 = machine.Pin(5, machine.Pin.IN, machine.Pin.PULL_DOWN)
esp32.wake_on_ext1(pins=(wake1,wake2), level=esp32.WAKEUP_ALL_LOW)
#HARDWARE INIT
i2c = I2C(scl=Pin(20), sda=Pin(21))
r = PCF8563(i2c)
lcd = SharpMemLCD()
#TIME SHOW
dd=str(r.date())
print(dd)
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
lcd.text(dd,0,110,0)
lcd.font_set(0x34,0,2,0)
lcd.text(hh,30,10,0)
lcd.text(mm,30,63,0)
lcd.show()
#CHECK SLEEP
wr = machine.wake_reason()
print(wr)
if wr==4:
    machine.deepsleep(59000)
print('go main')









