from machine import Pin, SPI
import time

class ColorMemLCD:
    LCD_COLOR_CMD_UPDATE_1BIT = 0x88
    LCD_COLOR_CMD_UPDATE_3BIT = 0x80
    LCD_COLOR_CMD_UPDATE_4BIT = 0x90
    LCD_COLOR_CMD_ALL_CLEAR = 0x20
    LCD_COLOR_CMD_NO_UPDATE = 0x00
    LCD_COLOR_CMD_BLINKING_WHITE = 0x18
    LCD_COLOR_CMD_BLINKING_BLACK = 0x10
    LCD_COLOR_CMD_INVERSION = 0x14

    def __init__(self, clk, mosi, ss):
        self._clk = Pin(clk, Pin.OUT)
        self._mosi = Pin(mosi, Pin.OUT)
        self._ss = Pin(ss, Pin.OUT)
        # SPI initialization
        self.spi = SPI(1, baudrate=8000000, polarity=0, phase=0)
        # Set pin state
        self._ss.value(1)
        # Initialize variables
        self.polarity = 0
        self.blink_cmd = 0x00
        self.disp_buf = bytearray(176 * 176 // 2)
        self._background = 0x0e
        self.clearDisplay()

    def toggle_vcom(self):
        self.polarity ^= 1

    def drawPixel(self, x, y, color):
        if x % 2 == 0:
            self.disp_buf[(176 // 2) * y + (x // 2)] &= 0x0F
            self.disp_buf[(176 // 2) * y + (x // 2)] |= (color & 0x0F) << 4
        else:
            self.disp_buf[(176 // 2) * y + (x // 2)] &= 0xF0
            self.disp_buf[(176 // 2) * y + (x // 2)] |= (color & 0x0F)

    def cls(self):
        for i in range(len(self.disp_buf)):
            self.disp_buf[i] = (self._background & 0x0F) | ((self._background & 0x0F) << 4)

    def refresh(self):
        self._ss.value(1)
        time.sleep_us(6)
        self.sendbyte(self.LCD_COLOR_CMD_UPDATE | (self.polarity << 6))
        self.toggle_vcom()
        for line in range(176):
            self.sendLineCommand(self.disp_buf[line * 88:(line + 1) * 88], line)
        time.sleep_us(6)
        self._ss.value(0)

    def sendLineCommand(self, line_cmd, line):
        if line < 0 or line >= 176:
            return
        time.sleep_us(6)
        self._ss.value(1)
        time.sleep_us(6)
        self.sendbyte(self.LCD_COLOR_CMD_UPDATE | (self.polarity << 6))
        self.toggle_vcom()
        self.sendbyte(line + 1)
        self.spi.write(line_cmd)
        self.sendbyteLSB(0x00)
        time.sleep_us(6)
        self._ss.value(0)

    def setBlinkMode(self, mode):
        if mode == 'NONE':
            self.blink_cmd = self.LCD_COLOR_CMD_NO_UPDATE
        elif mode == 'WHITE':
            self.blink_cmd = self.LCD_COLOR_CMD_BLINKING_WHITE
        elif mode == 'BLACK':
            self.blink_cmd = self.LCD_COLOR_CMD_BLINKING_BLACK
        elif mode == 'INVERSE':
            self.blink_cmd = self.LCD_COLOR_CMD_INVERSION
        else:
            self.blink_cmd = self.LCD_COLOR_CMD_NO_UPDATE
        self._ss.value(1)
        time.sleep_us(6)
        self.sendbyte(self.blink_cmd | (self.polarity << 6))
        self.toggle_vcom()
        self.sendbyteLSB(0x00)
        time.sleep_us(6)
        self._ss.value(0)

    def clearDisplay(self):
        for i in range(len(self.disp_buf)):
            self.disp_buf[i] = (self._background & 0x0F) | ((self._background & 0x0F) << 4)
        time.sleep_us(6)
        self._ss.value(1)
        time.sleep_us(6)
        self.sendbyte(self.LCD_COLOR_CMD_ALL_CLEAR | (self.polarity << 6))
        self.toggle_vcom()
        self.sendbyteLSB(0x00)
        time.sleep_us(6)
        self._ss.value(0)

    def sendbyte(self, data):
        self.spi.write(bytearray([data]))

    def sendbyteLSB(self, data):
        self.spi.write(b'\x00')

scr=ColorMemLCD(6,7,8)