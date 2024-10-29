# MicroPython driver for the JDI LPM013M126A 176x176 Color Memory LCD
# Author: [jd3096]

import machine
import time
import framebuf
from machine import SPI,Pin,PWM

#COLOR
LCD_COLOR_BLACK     = const(0x00)   
LCD_COLOR_BLUE      = const(0x02)   
LCD_COLOR_GREEN     = const(0x04)   
LCD_COLOR_CYAN      = const(0x06)   
LCD_COLOR_RED       = const(0x08)    
LCD_COLOR_MAGENTA   = const(0x0a)    
LCD_COLOR_YELLOW    = const(0x0c)    
LCD_COLOR_WHITE     = const(0x0e)    

# Commands for the LCD
LCD_COLOR_CMD_UPDATE_1BIT = 0x88
LCD_COLOR_CMD_UPDATE_3BIT = 0x80
LCD_COLOR_CMD_UPDATE_4BIT = 0x90
LCD_COLOR_CMD_ALL_CLEAR = 0x20

LCD_COLOR_CMD_NO_UPDATE = 0x00
LCD_COLOR_CMD_BLINKING_WHITE = 0x18
LCD_COLOR_CMD_BLINKING_BLACK = 0x10
LCD_COLOR_CMD_INVERSION = 0x14

LCD_WIDTH = 176
LCD_HEIGHT = 176

class LPM013M126A(framebuf.FrameBuffer):
    def __init__(self, spi, cs, bl,width=LCD_WIDTH, height=LCD_HEIGHT):
        self.bl=PWM(Pin(bl), freq=1000)
        self.bl.duty(0)
        self.spi = spi
        self.cs = Pin(cs,Pin.OUT)
        self.cs.value(0)
        self.width = width
        self.height = height
        self.polarity = 0
        self.blkcmd = 0x00
        self.colorcmd = 0x90
        self.buffer = bytearray((self.width * self.height) // 2)  # 4 bits per pixel
        super().__init__(self.buffer, self.width, self.height, framebuf.GS4_HMSB)
        # Clear the display buffer
        self.set_blink_mode('none')
        self.clear()
    
    def set_bl(self,lux):
        #between 0-1023
        if lux>1023:
            lux=1023
        if lux<0:
            lux=0
        self.bl.duty(lux)
        
    def toggle_vcom(self):
        pass
        #self.polarity ^= 0x40  # Toggle bit 6
        
    def clear(self):
        self.send_command(LCD_COLOR_CMD_ALL_CLEAR)
        
    def send_command(self, command):
        self.cs.value(1) 
        time.sleep_us(6)
        self.spi.write((command | self.polarity).to_bytes(1,'big'))
        self.toggle_vcom()
        self.spi.write(b'\x00')
        time.sleep_us(6)
        self.cs.value(0) 
        
    def show(self):
        # Send the buffer to the display
        self.cs.value(1) 
        time.sleep_us(6)
        self.spi.write((self.colorcmd | self.polarity).to_bytes(1,'big'))
        self.toggle_vcom()
        for y in range(self.height):
            line_address = y + 1
            self.spi.write(line_address.to_bytes(1,'big'))
            line_start = (self.width // 2) * y
            line_end = line_start + (self.width // 2)
            self.spi.write(self.buffer[line_start:line_end])
            self.spi.write(b'\x00')  # Dummy byte at the end of each line
        self.spi.write(b'\x00')  # Final dummy byte
        time.sleep_us(6)
        self.cs.value(0)  
           
    def set_blink_mode(self, mode):
        # Set the blinking mode of the display
        if mode == 'none':
            self.blkcmd = LCD_COLOR_CMD_NO_UPDATE
        elif mode == 'white':
            self.blkcmd = LCD_COLOR_CMD_BLINKING_WHITE
        elif mode == 'black':
            self.blkcmd = LCD_COLOR_CMD_BLINKING_BLACK
        elif mode == 'inverse':
            self.blkcmd = LCD_COLOR_CMD_INVERSION
        else:
            self.blkcmd = LCD_COLOR_CMD_NO_UPDATE
        self.send_command(self.blkcmd)
        
    def set_color_mode(self, mode):
        if mode == 1:
            self.colorcmd = LCD_COLOR_CMD_UPDATE_1BIT
        elif mode == 3:
            self.colorcmd = LCD_COLOR_CMD_UPDATE_3BIT
        elif mode == 4:
            self.colorcmd = LCD_COLOR_CMD_UPDATE_4BIT
        else:
            self.self.colorcmd = LCD_COLOR_CMD_UPDATE_4BIT
        
spi = SPI(baudrate=20000000,polarity=0, phase=0,firstbit=SPI.MSB,sck=Pin(6),mosi=Pin(7),miso=Pin(0))
lcd=LPM013M126A(spi,8,5)
import ufont
font = ufont.BMFont("mario.bmf")
lcd.set_bl(0)
city_buffer = bytearray(b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02@\x03@\x14@\x04 \x14333333333333333334DDDDDDDDDDDDDD333333333333334343D\x10C\x00B\x02R\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x121\x121#1\x13!\x133333333333333DC33DDDDDDDDDDDDDDD333C333333333333333\x113\x112\x122\x10\x01\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02A\x02A\x04A\x030\x04 \x140\x1333333333334DDDDDDDDDDDDDDDDDDDDDDDDDD33333333\x104\x10#\x003\x00C\x01B\x02A\x03A\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x012\x021\x020\x12!\x12323233333333DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDC33332323"32\x012\x022\x021\x020\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x122\x122\x121\x121\x13333333333344DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDCC3333333332\x112\x122\x121\x121\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01S\x01A\x02A\x04A\x030\x04 \x14333333DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDC3333C\x003\x00C\x01B\x02A\x03A\x03@\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\x00""""\x12!$B\x132#1$33333DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDC34333!3\x12C\x12C\x12!"""" \x02\x10\x00\x00\x00\x00\x00\x00\x014\x01B\x01B\x02A\x02C4C4C43333DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD333D3D3D3B\x02@\x02@\x03A\x030\x00\x00\x00\x00\x00\x003\x002\x012\x013330\x030\x1333334DDDDDDDDDDDDDDDEUUUUUUUUUUUUUUTDDDDDDDDDDDDDDDDD3333\x00B\x02C331\x020\x030\x140\x00\x00\x00\x00\x00"""\x12C\x11#C#B#1#3343DDDDDDDDDDDDDDDDEUUUUUUUUUUUUUUUDDDDDDDDDDDDDDDDDC433\x12B\x12C#B"#B\x12!" \x00\x00\x00\x00\x00\x00D\x00DDA\x02A\x02DC4C334DDDDDDDDDDDDDUUUUUUUUUUUUUUUUUUUUUUUUTDDDDDDDDDDDDDD33D3CB\x01@\x02DD@\x040\x00\x00\x00\x00\x00#2\x1133\x12C#3#33334DDDDDDDDDDDUDTEUUUUUUUUUUUUUUUUUUUUUUUUUDEEDDDDDDDDDDDDD3333#C#A\x13C!\x130\x00\x00\x00\x00#2\x003B\x013C3333334DDDDDDDDDDDUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUTDDDDDDDDDDDD3333C#C \x13C \x131\x00\x00\x00\x00\x00\x01D\x00DB\x01333334DDDDDDDDDDUUUUUUUUUUUUUEUUUUUUUUUUUUUUUUUUUUUUUUDDDDDDDDDDDD331\x03A\x02D@\x040\x04 \x00\x00#\x00"\x11"\x12C#3334DDDDDDDDDDEDUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUTEDDDDDDDDDDDC3#3#1\x12!#!\x13\x10\x00D\x103\x00B\x01CC3334DDDDDDDDDDUUUUUUUUUUUUUTUfffffffffffeeDUUUUUUUUUUUUUDDDDDDDDDDDD333D0\x030\x14@\x14 \x00\x00$D33333334DDDDDDDDEUUUUUUUUUUTEeUUUUUUUUUUUUUUUUUUUUUUEUUUUUUUUUUUDDDDDDDDDC330\x13334C \x00\x00\x114"33333334DDDDDDDDEUUUUUUUUUUUUeUUVUUUUUUUUUUUUUUeVUfeUUUUUUUUUUUUDDDDDDDDDD331\x13C32$!\x00\x004C\x00333334DDDDDDDDEUUUUUUUUUEffVeUUUUUUUUUUUUUUUUUUUUUUVfffeEUUUUUUUUUDDDDDDDDDC34C4 \x04D \x0043332\x01C3DDDDDDDEUUUUUUUUUEUUUUUUUUUUWwwwwwwwwwwwwwUUUUUUUUUUTUUUUUUUUUUDDDDDDDDD0\x03334 \x00\x0043332\x02C4DDDDDDDEUUUUUUUUUUfUUUUUfffegwwwwwwwwwwwwwUfffeUUUUUUUUUUUUUUUUDDDDDDDDD1\x03333!\x00\x004\x10#3333DDDDDDDEUUUUUUUTeeUUUUUUVwwwwwwwwwwwwwwwwwwwwwwvUUUUUVUfTUUUUUUUUDDDDDDDDC3330\x14 \x004!333DDDDDDDDDEUUUUUTUUeUUUVfffgwwwwwwwwwwwwwwwwwwwwwwwfffuUUUVUUUUUUUUUDDDDDDDDDD1\x142$ \x0133333DDDDDDDEUUUUUUUTVeUUffVwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwvVfeUVfUUUUUUUUTDDDDDDDE1\x1434!43333DDDDDDDEUUUUUUTVeUUUUwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwuUUUUfTUUUUUUTDDDDDDDDC0\x03D#3333DDDDDDDEUUUUUUUVUUUffwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwvfeUUVUUUUUUUTDDDDDDDDC1#3\x00333DDDDDDDUUUUUUTVfeUUUwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwweUUUVfUUUUUUUTDDDDDDDCD #33DDDDDDDUUUUUUTUUUUUgwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwweUUUUTUUUUUUTDDDDDD@\x133#33DDDDDDEUUUUUUUVeUffwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwffeUVUUUUUUUTDDDDDDA$3\x003DDDDDDDUUUUUTVfeUUgwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwweUUfeEUUUUUTDDDDDEU \x12DDDDDDDDUUUUUUUUUUVwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwveUUUUUUUUUTDDDDDDD2#DDDDDDDUUUUUUUeUVegwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwefUUfUUUUUUTDDDDDDE\x00DDDDDDUUUUUDEeUUgwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwUUUUDUUUUUTDDDDD \x13DDDDDDUUUUUUUeUVgwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwfUUeUUUUUUTDDDDD25DDDDDUUUUUEfeUUWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwUUUfeEUUUUTDDDDE5DDDDUUUUUEUUUWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwUUUUUUUUUDDDDE#DDDDUUUUUUeUVgwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwfUUeUUUUUTDDD3\x00DDDUUUUUEeUUWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwUUUeEUUUUDDD $DDEUUUUUUUUWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwvUUUUEUUUUTDD5DDUUUUUUeUUWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwvUUUeEUUUUTDE5DEUUUUEeUUWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwvUUUUEUUUUDE5DUUUUUUeUVgwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwfUUeUUUUUTE4UUUUUUfUUVwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwvUUVUEUUUUE5UUUUUUeUWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwuUUVeUUUUU5UUUUUeUVWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwveUVeUUUUU5UUUUEUUVwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwvUVUEUUUU5UUUTUUUwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwfUUUUUUU5UUUUfUfwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwvUVUUUUU5UUEfUVwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwuUVTUUU5UUUUVfwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwveVUUUU5UUVUVwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwvUVeUU5UEVUVwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwuUVTUU5UUUUfwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwveUUUU5DVUUwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwuUVTE5UVVfwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwweUUU5VUVwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwuUVU5VUVwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwuUVU6UUgwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwveUVFUUwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwuUVFUgwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwweVFUwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwuVFUwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwweVFwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwf6wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwf6wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwfGwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwgwwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwEwwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww5wwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwvffffffwwwwwwwwwwwwwwwwwwwwwwww5wwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwR333334wwwwwwwwwwwwwwwwwwwwwwww$wwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwuUUUUUC333333UVwwwwwwwwwwwwwwwwwwwwwU5wwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwws#3333333333334wwwwwwwwwwwwwwwwwwwwv#5wwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwb""""""#3"3#gwwwwwwwwwwwwwwwwwwwwv34wwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwtDDDDDD22D3Ewwwwwwwwwwwwwwwwwwwwwv35wwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwB!w4wwwwwwwwwwwwwwwwwwwwwwv35wwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwweUw3wwwwwwwwwwwwwwwwwwwTTET44FwwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww3wwwwwwwwwwwwwwwwwwwC333DC%wwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww3wwwwwwwwwwwwwwwwwwwC333335wwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwfwwwwwwwwwwwwwwwwwwwC333335wwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwC333335wwwwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwCDDDDD4D6wwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwvegwwwwwwwwwwwwwwwwwwwwwwwwwwwweC34433C35wwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwws37wwwwwwwwwwwwwwwwwwwwwwwwwwwwR333333335wwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwt3Gd3WwwwwwwwwwwwwwwwwwwwwwwwwwS3DDDD3335wwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwd3Fc3GwwwwwwwwwwwwwwwwwwwwwwwwwS3DDDD3335wwwwwwwwwwWwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwws34D34D3WwwwwwwwwwwwwwwwwwwwwwwwwS3DD3D3335wwwwwwwwwwWwwwwwwwwwwwwwwwtWwwwwwwwwwwwwwwwwws3333333WwwwwwwwwwwwwwwwwwwwwwwwwS3DD4D3335wwwwwwwwwwWwwwwwwwwwwwwwwwrGwwwwwwwwwwwwwwwwws3333333WwwwwwwwwwwwwwwwwwwwwwwwwS3DDDD3335wwwwwwwwwwWwwwwwwwwwwwwwwwsGwwwwwwwwwwwwwwwwt33333333WwwwwwwwwwwwwwwwwwwwwwwwwS3DDDD3335wwwwwwwwwwWwwwwwwwwwwwwwwwsGwwwwwwwwwwwwwwwvd33333333VwwwwwwwwwwwwwwwwwwwwwwwwS3DDDD3335wwwwwwwwwwWwwwwwwwwwwwwwwwsGwwwwwwwwwwwwwwwt#333333333gwwwwwwwwwwwwwwwwwwwwwwwS3DDDD3335wwwwwwwwwwWwwwwwwwwwwwwwuDCGwwwwwwwwwwwwwwwu4C4C34C4C3DgwwwwwwwwwwwwwwwwwwwwwwS3DDD33335wwwwwwwwwwWwwwwwwwwwwwwwt33Gwwwwwwwwwwwwwwwu3C4C34C4C33FwwwwwwwwwwwwwwwwwwwwwwS3DDD33335wwwwwwwwwwWwwwwwwwwwwwwwt33Gwwwwwwwwwwwwwwwu333333333333gwwwwwwwwwwwwwwwwwwwwwS33DDD33D6wwwwwwwwwwWwwwwwwwwwvUUUT33Egwwwwwwwwwwwwwwu333333333333gwwwwwwwwwwwwwwwwwwwwwS33DDD33D6wwwwwwwwwwWwwwwwwwwwt#333333Gwwwwwwwwwwwwwwu333333333333gwwwwwwwwwwwwwwwwwwwwwS3DDDD33C6wwwwwwwwwwWwCwwwwwwt33333333Wwwwwwwwwwwwwwwu4C334C4C43D3gwwwwwwwwwwwwwwwsWwwwwS3DDDD3335wwwwwwwwwwWwCEwwwwwt334CDDC3Wwwwwwwwwwwwwwwu3C334C4343D3gwwwwwwwwwvUUUWwsEgwwwS344DC3335wwwwwwwwwwWwC#wwwwwt334DDDC3Wwwwwwwwwwwwwwwu333333333333gwwwwwwwwwu#337ws2WwwwS333333335wwwwwwwwwwWwC4wwwwwt334DDDC3Wwwwwwwwwwwwwwwu333333333333gwwwwwwwwuD4DCGws3DWwwR3333333C6wwwwwwwwwwWwC4fwwwwt334DDDC3Wwwwwwwwwwwwwwwu333333333333gwwwwwwwwt34C3Ffc32GwwTC333334D6wwwwwwwwwwWwC33wwwwt33333333Wwwwwwwwwwwwwwwu34C333333333gwwwwwwwwt44C33333CWwwwC3DDD34D6wwwwwwwwww5D333wwwwt33333333Wwwwwwwwwwwwwwwu333333333333gwwwwwwwwt44C34333CWwwwCDDDD33C5wwwwwwwwww$3333wwwwt33333333Wwwwwwwwwwwwwwwu333333333333gwwwwwwwwt44C34C33CWwwwCDDDD3335wwwwwwwwww$4333wwwwwu3333333Wwwwwwwwwwwwwwwu4C34C3DC3D33gwwwwwwwwt44C34C33CWwwwCDD333335wwwwwwwwww$3333wwwwwu3333333Wwwwwwwwwwwwwwwu4C34C3433433gwwwwwwwwt44C34C33CWwwwCDDCC33C5wwwwwwwwww$3333wwwwwu3333333Wwwwwwwwwwwwwwwu333333333333gwwwwwwwwt44C34C33CWwwwCDDDD34D6wwwwwwwwww$3333wwwwwu3333333WwwwwwwCDD5wwwwu333333333333gwwwwwwwwt44C34C33CWwwwCDDDD34D6wwwwwwwwww$3333wwwwwu3333333WwwwwwfCDD5wwwwu3C3333333333gwwwwwwwwt44C34C33CWwwwCDDDD34D5ffgwwwwvww$3333wwwwwu3333333Wwwwww33335wwwwu4C4D34C3DC43gwwwwwwwwt44C3DC33CWwwwCC3DD333C336wwwwtDE$3333Vwwwwu333333CWwwwww3DC34UUwwu4C4C34C34333gwwwwwwwwt44C34C33CWeUUCDDDD333C336wwwwtDE$3333Ewwwwu3333C33Gwwwww3DC3334wwu33C333334343gwwwwwwwwt44C34C33CWTDDCDDDD3443336wwwwtUE$3333Ewwwwu334DC3CDDgwww3D333435wu33C343333333gwwwwwwvDD44DC4C33CDETUCDDDD34DD336wwwwtUU$4333Ewwwwu3C4DC4CEDgwww3DC443D5wu4D34DC4CDDD3gwwwwwfeETD4CCDCD3CEUUUCDDDD34DD4C6wwwvdUE$4333Ewwwwu3C4DC43EDgwww3DC3C4D5wu4DDDCCDCDDD3gwwwwvDDUT44D3DCDCCEUUUCC4DDD4DD4CFwwwdEUU$33D3EvEUUT344DDD3EDgwww3D33D4D5wu4C4DDCD3DDD3gwwwwvDUUT44C4DCDCCEUUUCDDDDDDDC4C6wuUTUUU$33D3EvDDDT3D4DDDCETVwww3D3DD4D5wu34DDDDDDCDD3gwwwwvDUUTD4D4DDDCCEUUUCDDDD4DDDDC6wuDEUUU$3343EvEUUT4DDDC4CEUDgww3DCDDDD5wu3CDDDC4DDDD3gwwwwvTUUTD4DDDCDDCEUUUCDDDDDDDDDDFwuEUUUU$4343UvEUUT4DDDDDCEUTVfw3DDDDDD5fd4DDDDDDDDDD3gwwwwvUUUTD4DDDCDDCEUUUCDDDDDDDDDCFwuEUUUU$DDD4EvEUUT4DDDDDCEUUDDg3DDDDDD4TT4DDDDDDDDDD3gwwwwvUUUT4DDDDCDDCEUUUCDDDDDDDDDC6wuEUUUU$DDDCCD4UUT4DDDDDCEUUUUw3DDDDDD4UT4DDDDDDDDDD3EUUUwvUUUT4DDDDDDCCEUUUCDDDDDDDDDC6wuEUUUV$4DDDD34UUT4DDDDDCEUUUUw3DDDD4D4UT4DDDDDDDDDDDUUUUwvUUUT4DDDDDDDCEUUDDDDDDDDDDDC6wuEUUUB$4DDDDD4UUT4DDDDDCEUUUUw3DDDDDD4UT4DDDDDDDDDDDUUUUwvEUUTDDDDDDDDCEUUCDDDDDDDDDDCFwuEUUU0$DDDDDD4UUT4DDDDDDDDEUUw3DDDDDD4UT4DDDDDDDDDD3UUUUwwvUUT4DDDDDDDCEUUCDDDDDDDDDDCFwuUUUUU$33334D4UUT4DDDDDDC3EUUw3DDDDDD4UT4DDDDDDDDDDDUUUUwwvUUT4DDDDDDDCEUUCDDDDDDDDDDCFwuUUUUV$33333D4UUd4DDDDDDDCEUUw3DDDDDD4UT4DDDDDDDDDDDUUUUwwvUUTDDDDDDDDCEUUCDDDDDDDDDDDFwuUUUUV4"""""D333C4DDDDDDDCEUUw3DDDDDD4UT4DDDDDDDDDDDUUUUUwUUUT4DDDDDDDCEUUCDDDDDDDDDDDDDDDDEUV4"""""D33333DDDDDDDCEUUw3DDDDDD4UT4DDDDDDDDDDDUUUUUwUUUT4DDDDDDDCEUUCDDDDDDDDDDDC34DDEUV$"""""3""""#DDDDDDDCEUUw4DDDDDDDUT4DDDDDDDDDDDUUUUUwUUUTDDDDDDDDCEUUCDDDDDDDDDDDDDDDDCF0#"""""2""""#DDDDDDDCEUUw4DDDDDD4UT4DDDDDDDDDDDUUUUUfUUUTDDDDDDDDCEUUCDDDDDDDDDDDDDDDDCFC\x12""""""""""#DDDDDDDCEUUw4DDDDDD4Ud4DDDDDDDDDDDUUUUUUUUUTDDDDDDDDCEUUCDDDDDDDDDDDDDDDDDEV\x12""""""""""#333334DDEUUw4DDDDDDDUd4DDDDDDDDDDDUUUUUUUUUTDDDDDDDDCEUUCDDDDDDDDDDDDDDDDDEV\x12""""""""""#333334DDEUUw4DDDDDDDUd4DDDDDDDDDDDUUUUUUUUUTDDDDDDDDCEUUCDDDDDDDDDDDDDDDDDFT\x12""""""""""""""""$DCEUUw4DDDDDDDUd4DDDDDDDDDDDUUUUUUUUUTDDDDDDDDDEUVDDDDDDDDDDDDDDDDDDF0\x00"""""""""""""""#DDDFUUw4DDDDDDEed4DDDDDDDCDDDUUUUUUUUUTDDDDDDDDDEUUDDDDDDDDDDDDDDDDDDFD\x01"""""""""""""""$DDDVUUw4DDDDDDEedDDDDDDDDB4DDUUUUUUUUUdDDDDDDDDDFUUDDDDDDDDDDDDDDDDDDEf#""""""""""""""""$DDVUUwDDDDDDDEedDDDDDDDD14DDVUUUfffffdDDDDDDDDDEUVDDDDDDDDDDDDDDDDDDFg\x12""""""""""""""""$DDVUUwDDDDDDDEedDDDDDDDD14DDVUUeUUUUUdDDDDDDDDDFUVTDDDDDDDDDDDDDDDDDFC\x00""""""""""""""""$DDVUUwDDDDDDDEedDDDDDDDD14DDUUUU4DDD5dDDDDDDDDDFUVTDDDDDDDDDDDDDDDDDF0\x12""""""""""""""""$DDVfUwDDDDDDDEedDDDDDDC4243343fDDDDDEdDDDDDDDDDDDVTDDDDDDDDDDDDDDE0\x04W@\x12""""""""""""""""4DDETUwDDDDDDDEedDDDDDDB#"#33""D3DDDDEdDDDDDDDDDDDVTDDDDDDDDDDDDDC$B%C \x00""""""""""""""""4DDDCVwDDDDDDDEedDDDDDEB"""""""333DDDEdDDDDDDDDDDDVTDDDDDDDDDDDDDA\x04UU \x00\x12""""""""""""""""$DDDDDUDDDDDDDEfeDDDDDC"""""""""""DDDEdDDDDDDDDDDDVTDDDDDDDDDDDDDD2$B# \x12""""""""""""""""4DDDDDDDDDDDDDEeUDDDDDA\x12"""""""""\x12DDDEdDDDDDDDDDDDVTDDDDDDDDDDDDDD0\x15@%0\x00""""""""""""""""4DDDDDDDDDDDDDEdDDDDDDB"""""""""""DDDEdDDDDDDDDDDDVTDDDDDDDDDS4DDA\x04DDU0\x12""""""""""""""""$DDDDDDDDDDDDDEdDDDDDDB"""""""""""DDDDTDDDDDDDDDDDVTDDDDDDDDDB#TDC4DB"$#""""""""""""""""$DDDDDDDDDDDDDEdDDDDDDB"""""""""""DDDDDDDDDDDDDDDDVTDDDDDDDDD2\x13TTEUD@\x00&\x00""""""""""""""""4DDDDDDDDDDDDDEdDDDDDEB"""""""""""DDDDDDDDDDDDDDDDVTDDDDDDDDS"$Q\x14U1%DD1\x00"\x11""""""""""""""$DDDDDDDDDDDDDEeDDDDDEB"""""""""""EDDDDDDDDDDDDDDDVTDDDDDDDDB\x12#A$UA%B50\x00"\x00\x12""""""""""""!$DDDDDDDDDDDDDEeDDDDDEB"""""""""""EDDDDDDDDDDDDDDDVTDDDD33331\x02"$UUUU@\x150\x00""""!\x01"""""""""$DDDDDDDDDD3333333333332"""""""""""4DDDDDDDDDDDDDDDVTDDDT""""""1\x04Q\x12"""\x11\x00\x00""""!\x01"""""""""$DDDDDDDDDD3333333333332"""""""""""3DDDDDDDDDDDDDDDVTDDDT\x12"""""!\x03@\x02""#\x10\x00\x00#"\x00""""""""""$TDDDDDDDDDD"""""""""""""""""""""""""#DDDDDDDDDDDDDDDVTDDDT"!\x01""0\x012" \x02 \x03\x10\x00\x00\x11"\x11""""""""""$DDDDDDDDDDD"""""""""""""""""""""""""#UDDDDDDDDDDDDDDVTDDT3""\x12""!\x12!\x12!\x11\x11\x11\x00\x00\x00\x00#"""""""""""$DDDDDDDDDDD"""""""""""""""""""""""""#UDDDDDDDDDDDDDDVTDDT"""""""" \x02# \x03 \x00\x00\x003\x10"\x002\x012""""""$TDDDDDDDDDD"""""""""""""""""""""""""#UDDDDDDDDDDDDDUfTUTT"""!\x01"""# \x03 \x00\x00\x00\x00\x00"\x00\x12\x11!\x112\x12"""""$DDDDDDDDDDD"""""""""""""""""""""""""#UDDDDDDDDDDDDUDUDDUT""""\x11"\x12"\x12 \x02\x11\x00\x01\x00\x00\x00\x00\x00\x003\x00""\x00"""""$TDDDDDDDDDD"""""""""""""""""""""""""\x13UDDDDDDDDDUDEUC333UT\x00"""" \x021\x02 \x00\x03 \x03 \x00\x00\x00\x12"\x00"!\x00""""""$TDDDDDDDDDD"""""""""""""""""""""""""DTDDDDTTDEUUTEB""""22\x12""!\x012"2\x10\x02" \x12 \x00\x00\x00\x00\x12"\x11""\x112\x12"\x12""$TDDDDDDDDDD"""""""""""""""""""""""""UDDDTDTDETEDEEA"""""""""!\x11""" \x022\x10\x02 \x00\x00\x00\x00\x00\x003\x00##1\x011\x01""%TEUUUUUDDDD"""""""""""""""""""""""""UETEUUUUUUUUUUB""""""""""1\x00\x00\x020\x030\x00\x00\x00\x00\x00\x00\x00\x00\x11\x11!\x112\x11\x12"\x12!\x12"5TDDDDDDDDT""""""""""""""""""""""""\x12UEUETDDDDDDDDD2""!""\x11"\x11"\x122\x10\x01!\x11!\x12\x10\x00\x00\x00\x00\x00\x00\x00"\x002\x00!\x01""" \x02 \x15S33332EDDD""""""""""""""""""""""""\x12UUUUS3333333332""\x10\x12"\x00"\x001\x01"!\x021\x02 \x02 \x00\x00\x00\x00\x00\x00\x00\x013\x002\x011\x021\x012"4UR""""!EDDDT"""""""""""""""""""""""\x12UUUT2"""""""""""#""""""2"1\x010\x020\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x11\x00\x11!\x11\x11\x11\x11\x12!\x132"!\x12""!ETEDT""""""""""""""""""""""""UUUS""""""""""""\x11""\x11"\x112\x112\x11\x11\x10\x01\x11\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01B\x011\x020\x020\x02 \x03\x10\x13""!ETEDT""""""""""""""""""""""""UUUS""""""""""""\x00\x13"\x00"\x002\x001\x011\x00\x00\x020\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x11!\x12!\x11!\x12!\x12""""""4EUDT""""""""""""""""""""""""UUUS""""""""""""""""""\x11!\x11!\x11 \x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00!\x01!\x01 \x01 \x12"""!""#EUDT""""""""""""""""""""""""UUUS"""""""""""!#!"""!\x00!\x01!\x01 \x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x011\x020\x030\x02 \x03 \x13 \x12""EUUT""""""""""""""""""""""""UUUS"""""""""""\x00#\x10#\x00"\x002\x001\x011\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x11!\x12!\x12!\x12"""""43CC""""""""""""""""""""""""UUUS""""""""""""""\x11"\x11"\x11!\x10\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x020\x020\x030\x03"#"""3333""""""""""""""""""""""""UUUS"""""""""""""3\x003\x002\x002\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03 \x13 \x13\x10#\x103\x10"\x002\x012""""""""""""""""""""UUUT""# \x02 \x03 \x13\x10\x13\x10#\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x10\x01\x10\x01\x00\x11\x00\x11\x00\x11\x00\x11\x00\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x113332\x11\x11\x11\x10\x01\x10\x01\x10\x01\x00\x01\x00\x11\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')
city_fb = framebuf.FrameBuffer(city_buffer, 176, 176, framebuf.GS4_HMSB)
lcd.blit(city_fb,0,0)
font.text(lcd, "21:48", 45, 35, color_type=1,font_size=32, color=9, bg_color=6, show=True, clear=True)
lcd.show()



