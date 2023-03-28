from LCD import MemLCD # MemLCD
import time

lcd = MemLCD()
lcd.write("Hello World", 0, 0)
lcd.sync()

time.sleep(1)
lcd.write("Sharp-MicroPython", 0, 10)
lcd.sync()

time.sleep(1.5)
lcd.horizontal_line(0, lcd.ydim//2, lcd.xdim)
lcd.vertical_line(lcd.xdim//2, 20, lcd.ydim)
lcd.sync()

time.sleep(0.5)
lcd.solid_rectangle(0, lcd.ydim//2 - 20, 10, 10)
lcd.empty_rectangle(lcd.xdim//2 + 20, lcd.ydim//2-20, 10, 30)
lcd.sync()

time.sleep(3)
lcd.clear_screen()
lcd.clear_buffer()
lcd.write("Demo Over", 36, 80)
lcd.sync()