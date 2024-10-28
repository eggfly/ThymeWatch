from sharp_mem_lcd import SharpMemLCD
import time
import machine
machine.freq(160000000)

lcd = SharpMemLCD()
# lcd.text("Hello World", 0,0,1)
# lcd.show()
# 
# time.sleep(1)
# lcd.text("Sharp-MicroPython", 0, 10,1)
# lcd.show()
# 
# time.sleep(1.5)
# lcd.hline(0, lcd.ydim//2, lcd.xdim,1)
# lcd.vline(lcd.xdim//2, 20, lcd.ydim,1)
# lcd.show()
# 
# time.sleep(0.5)
# lcd.rect(0, lcd.ydim//2 - 20, 10, 10,1)
# lcd.rect(lcd.xdim//2 + 20, lcd.ydim//2-20, 10, 30,1,1)
# lcd.show()
# 
# time.sleep(3)
# lcd.clear()
# lcd.clear_buffer()
# lcd.text("Demo Over", 36, 80,1)
# lcd.show()

cube=[[-15,-15,-15],[-15,15,-15],[15,15,-15],[15,-15,-15],[-15,-15,15],[-15,15,15],[15,15,15],[15,-15,15]]
lineid=[1,2,2,3,3,4,4,1,5,6,6,7,7,8,8,5,8,4,7,3,6,2,5,1]


def matconv(a,matrix):
    res=[0,0,0]
    for i in range(0,3):
        res[i]=matrix[i][0]*a[0]+matrix[i][1]*a[1]+matrix[i][2]*a[2]
    for i in range(0,3):
        a[i]=res[i]
    return a

def rotate(obj,x,y,z):
    x=x/pi
    y=y/pi
    z=z/pi
    rz=[[cos(z),-sin(z),0],[sin(z),cos(z),0],[0,0,1]]
    ry=[[1,0,0],[0,cos(y),-sin(y)],[0,sin(y),cos(y)]]
    rx=[[cos(x),0,sin(x)],[0,1,0],[-sin(x),0,cos(x)]]
    matconv(matconv(matconv(obj,rz),ry),rx)
    
def drawcube(x,y,z):
    lcd.fill(1)
    for i in range(0,8):
        rotate(cube[i],x,y,z)
    for i in range(0,24,2):
        x1=int(64+cube[lineid[i]-1][0])
        y1=int(32+cube[lineid[i]-1][1])
        x2=int(64+cube[lineid[i+1]-1][0])
        y2=int(32+cube[lineid[i+1]-1][1])
        lcd.line(x1,y1,x2,y2,0)
        #print(64+cube[lineid[i]-1][0],32+cube[lineid[i]-1][1],64+cube[lineid[i+1]-1][0],32+cube[lineid[i+1]-1][1])
    lcd.show()
    
from math import cos,sin,pi
while 1:
    drawcube(0.1,0.2,0.3)
