from machine import I2C,Pin,RTC
from drivers import PCF8563
import time

def esp2rtc():
    try:
        i2c = I2C(scl=Pin(20), sda=Pin(21))
        r = PCF8563(i2c)
        print('rtc time')
        print(r.datetime())
        time.sleep(0.2)
        print('sync system to pcf8563')
        r.write_now()
        return True
    except:
        return False
    
def rtc2esp():
    try:
        i2c = I2C(scl=Pin(20), sda=Pin(21))
        r = PCF8563(i2c)
        print('rtc time')
        print(r.datetime())
        #(year, month, day, weekday, hours, minutes, seconds, subseconds)
        esptime=(r.year()+2000,r.month(),r.date(),r.day(),r.hours(),r.minutes(),r.seconds(),0)
        print(esptime)
        self.rtc.datetime(esptime)
        print('esp32 time')
        print(self.rtc.datetime())
        return True
    except:
        return False
rtc=RTC()
print(rtc.datetime())
print(time.localtime())
esp2rtc()
