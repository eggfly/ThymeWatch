from machine import SPI, Pin
import framebuf,time

class MemLCD:
    XDIM = 144
    YDIM = 168

    def __init__(self):
        self.spi = SPI(baudrate=1100000,polarity=0, phase=0,firstbit=SPI.MSB, sck=Pin(1), mosi=Pin(13),
                       miso=Pin(8))
        self.cs = Pin(12, Pin.OUT)
        self.cs.value(0)
        #self.disp = Pin(self.DISP, Pin.OUT)
        #self.disp.value(1)

        self._xdim = self.XDIM
        self._ydim = self.YDIM
        self.buffer = bytearray((self._xdim//8) * self._ydim)
        self.framebuffer = framebuf.FrameBuffer(self.buffer, self._xdim, self._ydim, framebuf.MONO_HMSB)
        self.vcom = 2
        # self.clear_screen()

    @property
    def xdim(self):
        return self._xdim

    @property
    def ydim(self):
        return self._ydim

    def toggle_vcom(self):
        self.vcom ^= 2

    # send lcd command to clear screen
    def clear(self):
        self.cs.value(1)
        time.sleep_us(6)          
        self.send(4 | self.vcom)
        self.send(0)
        time.sleep_us(2)          
        self.cs.value(0)
        time.sleep_us(6)          
        self.toggle_vcom()


    def send(self, value):
        self.spi.write(value.to_bytes(1, 'little'))

    # sync up screen with framebuffer contents
    # done by implementing the lcd command to write a single line, for every line in the buffer
    def show(self):
        index = 0
        for line in range(self._ydim):
            self.cs.value(1)
            time.sleep_us(6)       
            self.send(1 | self.vcom)
            self.send(line+1)
            for j in range(self._xdim//8):
                self.send(self.buffer[index])
                index += 1
            self.send(0)
            time.sleep_us(2)     
            self.cs.value(0)
            time.sleep_us(6)              