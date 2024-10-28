from LCD import MemLCD
import time

lcd = MemLCD()
lcd.framebuffer.text("Hello World", 0, 0, 1)
lcd.show()

time.sleep(1)
lcd.framebuffer.text("Sharp-MicroPython", 0, 10, 1)
lcd.show()

time.sleep(1.5)
lcd.framebuffer.hline(0, lcd.ydim//2, lcd.xdim,1)
lcd.framebuffer.vline(lcd.xdim//2, 20, lcd.ydim,1)
lcd.show()

time.sleep(0.5)
lcd.framebuffer.rect(0, lcd.ydim//2 - 20, 10, 10,0)
lcd.framebuffer.rect(lcd.xdim//2 + 20, lcd.ydim//2-20, 10, 30,1)
lcd.show()

time.sleep(3)
lcd.clear()
lcd.framebuffer.text("Demo Over", 36, 80, 1)
lcd.show()