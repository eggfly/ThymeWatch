import machine
import time
from machine import SPI
from machine import Pin,PWM
bl=PWM(Pin(5), freq=1000)
bl.duty(0)

# SPI configuration
spi = SPI(baudrate=20000000,polarity=0, phase=0,firstbit=SPI.MSB,sck=Pin(6),mosi=Pin(7),miso=Pin(0))
cs = Pin(8,Pin.OUT)

cs.value(1)
time.sleep_us(5)
spi.write(b'\x20\x00') # ALL CLEAR MODE
cs.value(0)
time.sleep_us(5)


#Color Bar
cmd_buff = bytearray(90)
cmd_buff[0] = 0x90  #'1x01xxyy=4bit mode,
cmd_buff[1] = 0x00  #Line
for i in range(88):
    k=i>>4
    cmd_buff[i+2] = (k<<5)&0xe0 | (k<<1)&0xe #R0,G0,B0,D0,R1,G1,B1,D1

#Buffer->MIP
cs.value(1)
time.sleep_us(5)
for j in range(240):
    cmd_buff[1] = j  #line(M128A=240lines)
    resp = spi.write(cmd_buff)    
time.sleep_us(5)
cs.value(0)
time.sleep(0.1)

# cs.value(1)
# time.sleep_us(5)
# spi.write(b'\x10\x00') # ALL CLEAR MODE
# cs.value(0)
# time.sleep_us(5)