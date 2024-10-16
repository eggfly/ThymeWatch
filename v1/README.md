# ThymeWatch
ESP32-C3 Smart Watch with SHARP Memory Display


## IO Pins

ESP32-C3FN4 with 4MB internal QIO Flash

| Function | IO Pin | 备注 |
| -- | -- | -- |
| RTC_Interrupt | IO0 | PCF8563 |
| IMU_Interrupt | IO1 | BMI270 的 INT0 |
| VBAT_DIV | IO2 | 两个470K电阻给电池正极做分压，是电池电压的一半 |
| Buzzer | IO3 | 蜂鸣器 |
| BUTTON_UP | IO4 | 按下是低电平，已外部上拉 |
| BUTTON_DOWN | IO5 | 按下是低电平，已外部上拉 |
| LCD_SCLK | IO6 | 屏幕 SPI: 时钟 |
| LCD_SI | IO7 | 屏幕 SPI: MOSI |
| LCD_CS | IO8 | 屏幕 SPI: CS |
| BUTTON_MIDDLE (Boot) | IO9 | 启动模式选择，按下是低电平，内部弱上拉，额外有外部上拉 |
| LCD_DISP | IO10 | 屏幕 DISP: 高电平显示、低电平不显示（已外部上拉） |
| USB D- | IO18 | 已连到 USB Type-C |
| USB D+ | IO19 | 已连到 USB Type-C |
| I2C_SCL | IO20 | 原来是 U0RXD，现在是 SCL，外部上拉 |
| I2C_SDA | IO21 | 原来是 U0TXD，现在是 SDA，外部上拉 |


## I2C Addresses



| Device | Address | 备注 |
| -- | -- | -- |
| SHT30 | 68 (0x44) | 高精度温度/湿度传感器 |
| PCF8563 | 81 (0x51) | RTC 时钟，锂电池供电+两个大电容维持供电 |
| BMI270 | 104 (0x68)| 6轴惯性传感器（加速度&陀螺仪） |
| BMP280 | 118 (0x76)| 高精度气压/温度传感器 |


## 理论功耗

| Product | Processor | Cores |Frequency | Modem-sleep | Light-sleep | Deep-sleep |
| --|--|--| - | -| -| - |
| ESP8266 | Tensilica Xtensa® 32-bit L106 RISC| 1| 80 MHz (default) or 160 MHz | 15 mA | 0.9 mA| 20 µA |
| ESP32 | Tensilica Xtensa® single/dual-core 32-bit LX6 | 2 | Up to 240 MHz (160 MHz for ESP32-S0WD)| 20 ~ 68 mA | 0.8 mA | RTC timer + RTC memory: 10 µA<br>RTC timer only: 5 µA|
| ESP32-S2 | Xtensa® single-core 32-bit LX7 microprocessor | 1 | Up to 240 MHz | 10 ~ 23 mA | 750 µA | ULP sensor-monitored pattern: 22 µA<br>RTC timer + RTC memory: 25 µA<br>RTC timer only: 20 µA |
| ESP32-S3 | Xtensa® dual-core 32-bit LX7 microprocessor | 2 | Up to 240 MHz | 13.2 ~ 91.7 mA | 240 µA | RTC memory and RTC peripherals: 8 µA<br>RTC memory, RTC peripherals are powered down: 7 µA |
| ESP32-C3 | 32-­bit RISC-­V single-­core processor | 1 | Up to 160 MHz | 13 ~ 23 mA | 130 µA | RTC timer + RTC memory: 5 µA |

## 实际功耗
Deepsleep: 15.1 µA (通过测试，上下按钮的上拉电阻几乎不额外耗电)
Wake up: ~ 20 mA

* Serial.begin() 消耗 115 µs

## 关于 RTC Interrupt

* TODO: 接了 RTC Interrupt 后，IO0 内部弱上拉电阻不起作用，IO0电平一直是0，所以后续考虑使用 ESP32 的 RTC 作为闹钟唤醒

## 关于启动速度

* 在 setup() 或者app_main() 函数里使用 millis() 函数来探测第一行代码的消耗时间，消耗时间主要在 esp32 的 bootloader，并且默认app代码分区越大，bootloader验证分区数据完整性所需要的时间越长。
* 但是！deepsleep后重新唤醒默认是不需要再经过验证的，这块代码可以去看idf的bootloader实现。
* 测试数据：2MB大程序需要的验证时间大约是200ms+，去掉验证只需要40ms左右。
* 如果想要第一次也跳过验证，需要加入这一行宏：
CONFIG_BOOTLOADER_SKIP_VALIDATE_ALWAYS=y

## 续航计算

https://www.digikey.cn/zh/resources/conversion-calculators/conversion-calculator-battery-life

## 电池电压以及ADC计算公式

```C++
// map() 的浮点数版本
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  float result;
  result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  return result;
}

// 读取ADC并map()
float vbat = mapf(sensorValue, 0, 4095, 0, 3.3) * 2;
// 拟合公式
float real_vbat = 0.8681 * vbat + 0.0987;
// real_vbat 大于 4.20V 认为充电已充满

// 当然上面两个行计算代码，可以合并成一个
```
