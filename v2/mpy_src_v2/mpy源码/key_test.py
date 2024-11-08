from machine import Pin
touch=Pin(0,Pin.IN,Pin.PULL_UP)
import time

while 1:
    time.sleep(0.1)
    print(touch.value())