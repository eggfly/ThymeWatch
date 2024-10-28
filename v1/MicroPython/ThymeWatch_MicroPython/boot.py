

from sharp_mem_lcd import SharpMemLCD
import time
import machine
# machine.freq(160000000)

lcd = SharpMemLCD()

lcd.fill(0xff)
lcd.text('MicroPython!', 10, 10, 0x00)
lcd.text('%s' %time.ticks_ms(), 20, 40, 0x00)
lcd.text('%s' %time.ticks_us(), 20, 60, 0x00)
lcd.text('%s' %time.time_ns(), 20, 80, 0x00)
lcd.show()
