#!/usr/bin/env python3

def calc_battery_life_in_days(battery_mah, wakeup_ma, deep_sleep_ua, wakeup_seconds, deep_sleep_seconds):
    battery_uah = battery_mah * 1000.0
    average_ua = (wakeup_seconds * wakeup_ma * 1000.0 + deep_sleep_seconds * deep_sleep_ua) / (wakeup_seconds + deep_sleep_seconds)
    life_in_hours = battery_uah / average_ua
    life_in_days = life_in_hours / 24.0
    return life_in_days

# print(calc_battery_life_in_days(120, 30.0, 76.8, 1.0, 59.0))

print('30mAh+MicroPython理论续航天使数')
print(calc_battery_life_in_days(30, 20.0, 17.0, 0.399, 59.6))

print('30mAh+Arduino理论续航天使数')
print(calc_battery_life_in_days(30, 20.0, 17.0, 0.1, 59.9))

# print('开0秒，deep-sleep所有时间')
# print(calc_battery_life_in_days(120, 11.0, 20.0, 0.0, 59.0))


