
import time
from bmi270 import BMI270
from machine import Pin, SPI, I2C

# Init in I2C mode.
imu = BMI270(I2C(0, scl=Pin(20), sda=Pin(21)))



while (True):
    print('Accelerometer: x:{:>6.3f}  y:{:>6.3f}  z:{:>6.3f}'.format(*imu.accel()))
    print('Gyroscope:     x:{:>6.3f}  y:{:>6.3f}  z:{:>6.3f}'.format(*imu.gyro()))
    print('Magnetometer:  x:{:>8.3f}  y:{:>8.3f}  z:{:>8.3f}'.format(*imu.magnet()))
    print("")
    time.sleep_ms(100)
