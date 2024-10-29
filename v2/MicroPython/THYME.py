#PIN
TP_INT = const(0)
IMU_INT = const(1)
RTC_INT = const(2)
BUTTON_UP = const(4)
LCD_BL = const(5)
LCD_SCK = const(6)
LCD_SI = const(7)
LCD_CS = const(8)
BUTTON_DOWN = const(9)
TP_RST = const(10)
I2C_SCL = const(20)
I2C_SDA = const(21)

#BACKLIGHT(PWM)
from machine import Pin,PWM
bl=PWM(Pin(LCD_BL), freq=1000)
bl.duty(0)
#I2C
from machine import SoftI2C
i2c=SoftI2C(scl=I2C_SCL,sda=I2C_SDA)
print(i2c.scan())

# #BMP280
# #https://github.com/dafvid/micropython-bmp280
# from bmp280 import *
# bmp = BMP280(i2c)
# 
# bmp.use_case(BMP280_CASE_WEATHER)
# bmp.oversample(BMP280_OS_HIGH)
# bmp.temp_os = BMP280_TEMP_OS_8
# bmp.press_os = BMP280_PRES_OS_4
# bmp.standby = BMP280_STANDBY_250
# bmp.iir = BMP280_IIR_FILTER_2
# bmp.spi3w = BMP280_SPI3W_ON
# 
# bmp.force_measure()
# bmp.normal_measure()
# bmp.sleep()
# 
# print(bmp.temperature)
# print(bmp.pressure)

# #BMI270
# #https://github.com/jposada202020/MicroPython_BMI270/blob/master/examples/bmi270_simpletest.py
# import time
# from machine import Pin, I2C
# from micropython_bmi270 import bmi270
# 
# print(i2c.scan())
# bmi = bmi270.BMI270(i2c)
# 
# while True:
#     accx, accy, accz = bmi.acceleration
#     print(f"x:{accx:.2f}m/s2, y:{accy:.2f}m/s2, z{accz:.2f}m/s2")
#     time.sleep(0.1)
#     gyrox, gyroy, gyroz = bmi.gyro
#     print("x:{:.2f}°/s, y:{:.2f}°/s, z{:.2f}°/s".format(gyrox, gyroy, gyroz))
#     time.sleep(0.1)

# #DS3231
# #https://github.com/balance19/micropython_DS3231/blob/main/my_lib/RTC_DS3231.py
# import ds3231
# import time
# 
# rtc = ds3231.RTC(i2c)
# # It is encoded like this: sec min hour week day month year.
# #rtc.DS3231_SetTime(b"\x00\x45\x15\x03\x28\x10\x24")
# while True:
#     t = rtc.DS3231_ReadTime(1)  # Read RTC and receive data in Mode 1 (see /my_lib/RTC_DS3231.py)
#     print(t)
#     time.sleep(1)

# #SHT30
# #https://github.com/rsc1975/micropython-sht30/blob/master/sht30.py
# from sht30 import SHT30
# sensor = SHT30(i2c)
# temperature, humidity = sensor.measure()
# print('Temperature:', temperature, 'ºC, RH:', humidity, '%')

# #OPT3001
# #https://github.com/octaprog7/opt3001/blob/master/opt3001mod.py
# # import sys
# from machine import I2C, Pin
# from sensor_pack_2.bus_service import I2cAdapter
# import opt3001
# import time
# 
# def show_header(info: str, width: int = 32):
#     print(width * "-")
#     print(info)
#     print(width * "-")
# 
# def delay_ms(val: int):
#     time.sleep_ms(val)
# 
# 
# adapter = I2cAdapter(i2c)
# 
# als = opt3001.OPT3001(adapter)
# _id = als.get_id()
# print(f"manufacturer id: 0x{_id.manufacturer_id:x}; device id: 0x{_id.device_id:x}")
# 
# show_header("Однократный режим работы! Запуск измерения вручную! Автоматический выбор предела измерения датчиком!")
# als.long_conversion_time = False
# als.start_measurement(continuously=False, lx_range_index=10, refresh=False)
# cycle_time = als.get_conversion_cycle_time()
# print(f"cycle time мс: {cycle_time}")
# print(als.read_config_from_sensor(return_value=True))
# _repeat_count = 10
# for _ in range(_repeat_count):
#     delay_ms(cycle_time + 50)
#     ds = als.get_data_status()
#     if ds.conversion_ready:
#         # обработанные данные!
#         value = als.get_measurement_value(value_index=1)
#         print(value)
#     else:
#         print(f"Данные не готовы для считывания!")
#     # запуск измерения вручную
#     als.start_measurement(continuously=False, lx_range_index=12, refresh=False)
# 
# show_header("Автоматический запуск измерения! Автоматический выбор предела измерения датчиком!")
# als.long_conversion_time = True
# als.start_measurement(continuously=True, lx_range_index=12, refresh=False)
# print(als.read_config_from_sensor(return_value=True))
# cycle_time = als.get_conversion_cycle_time()
# print(f"cycle time мс: {cycle_time}; увеличенное время преобразования: {als.long_conversion_time}")
# for _ in range(10 * _repeat_count):
#     delay_ms(cycle_time)
#     ds = als.get_data_status()
#     if ds.conversion_ready:
#         # обработанные данные!
#         value = als.get_measurement_value(value_index=1)
#         print(value)
#     else:
#         print(f"Данные не готовы для считывания!")

# #INA219
# #https://github.com/robert-hh/INA219
# from machine import I2C
# from ina219 import INA219
# import time
# 
# ina = INA219(i2c)
# 
# ina.set_calibration_32V_1A()
# 
# while True:
#     current = ina.current
#     voltage = ina.bus_voltage
#     print("{} mA  {} V".format(current, voltage))
#     time.sleep(1)
