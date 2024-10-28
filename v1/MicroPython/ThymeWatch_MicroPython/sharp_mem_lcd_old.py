import machine
from machine import SPI, Pin, Timer
from utime import sleep_us


import framebuf

class SharpMemLCD:
    '''
    MicroPython driver to write to Sharp's Memory LCD. Should works with any standard pyboard
    Uses the built-in framebuffer class to format the buffer 
    Custom parameters (display pin, cs pin, screen's x and y dimensions, SPI channel) can be changed below
    '''
    CS = 8
    DISP = 10
    XDIM = 128
    YDIM = 128
    CHANNEL = 1
    BAUDRATE = 1100000

    def __init__(self):
        # define pins
        self.pin_sck = Pin(6, Pin.OUT)
        self.pin_mosi = Pin(7, Pin.OUT)
        # pin_miso = machine.Pin(19)
        self.spi = SPI(self.CHANNEL, self.BAUDRATE, polarity=0, phase=0, firstbit=SPI.MSB, sck=self.pin_sck, mosi=self.pin_mosi)
        self.cs = Pin(self.CS, Pin.OUT)
        self.set_cs(0)
        self.disp = Pin(self.DISP, Pin.PULL_UP)
        self.disp.value(1)

        # define structures, related info
        self._xdim = self.XDIM
        self._ydim = self.YDIM
        self.buffer = bytearray((self._xdim//8) * self._ydim)
        self.framebuffer = framebuf.FrameBuffer(self.buffer, self._xdim, self._ydim, framebuf.MONO_HMSB)
        self.vcom = 2

        # begin by clearing screen
        self.clear_screen()

    @property
    def xdim(self):
        return self._xdim

    @property
    def ydim(self):
        return self._ydim

    def clear_buffer(self):
        self.framebuffer.fill(0)

    # format framebuffer with text, starting at the x,y coords
    # each character is an 8 by 8 pixel array
    # written on the next line
    def write(self, text, x, y):
        self.framebuffer.text(text, x, y)

    # draw horizontal line in the buffer
    def horizontal_line(self, x, y, length):
        self.framebuffer.hline(x, y, length, 1)

    # draw vertial line in the buffer
    def vertical_line(self, x, y, height):
        self.framebuffer.vline(x, y, height, 1)

    # draw empty rectangle in the buffer (i.e. just the outline)
    def empty_rectangle(self, x, y, width, height):
        self.framebuffer.rect(x, y, width, height, 1)

    def solid_rectangle(self, x, y, width, height):
        self.framebuffer.fill_rect(x, y, width, height, 1)

    def toggle_vcom(self):
        self.vcom ^= 2

    # send lcd command to clear screen
    def clear_screen(self):
        self.set_cs(1)
        sleep_us(6)
        self.send(4 | self.vcom)
        self.send(0)
        sleep_us(2)
        self.set_cs(0)
        sleep_us(6)
        self.toggle_vcom()

    def set_cs(self, logic_level):
        self.cs.value(logic_level)

    def send(self, value):
        print(type(value))
        self.spi.write(chr(value))

    # sync up screen with framebuffer contents
    # done by implementing the lcd command to write a single line, for every line in the buffer
    def sync(self):
        index = 0
        for line in range(self._ydim):
            self.set_cs(1)
            sleep_us(6)       # tsSCS
            self.send(1 | self.vcom)
            self.send(line+1)
            for j in range(self._xdim//8):
                self.send(self.buffer[index])
                index += 1
            self.send(0)
            sleep_us(2)       # thSCS
            self.set_cs(0)
            sleep_us(6)       # twSCSL

def main():
    lcd = SharpMemLCD()
    # lcd.clear_screen()
    # lcd.sync()

if __name__ == '__main__':
    main()