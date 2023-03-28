#pragma once

const PROGMEM uint8_t unlock_xbm_8x11[] = {
    0x3C, 0x7E, 0x66, 0x06, 0xFF, 0xFF, 0xFF, 0xC3, 0xFF, 0xFF, 0xFF,
};

const PROGMEM uint8_t unlock_xbm_10x13[] = {
    0x78, 0x00, 0xFC, 0x00, 0xCE, 0x01, 0x86, 0x01, 0x06,
    0x00, 0x06, 0x00, 0xFF, 0x03, 0xFF, 0x03, 0xFF, 0x03,
    0xFF, 0x03, 0xFF, 0x03, 0xFF, 0x03, 0xFF, 0x03,
};

const PROGMEM uint8_t arrow_right_7x8[] = {0x80, 0x90, 0xb0, 0xff,
                                           0xff, 0xb0, 0x90, 0x80};

const PROGMEM uint8_t battery_100_23x12[] = {
    0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0x1F, 0x03, 0x00, 0x18, 0xBB, 0xBB, 0x1B,
    0xBB, 0xBB, 0x7B, 0xBB, 0xBB, 0x7B, 0xBB, 0xBB, 0x7B, 0xBB, 0xBB, 0x7B,
    0xBB, 0xBB, 0x1B, 0x03, 0x00, 0x18, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0x1F,
};

const PROGMEM uint8_t battery_charging_23x12[] = {
    0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0x1F, 0x03, 0x00, 0x18, 0x03, 0x0E, 0x18,
    0x03, 0x7E, 0x78, 0x03, 0xFE, 0x79, 0xE3, 0x1F, 0x78, 0x83, 0x1F, 0x78,
    0x03, 0x1C, 0x18, 0x03, 0x00, 0x18, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0x1F,
};

const PROGMEM uint8_t pref_20x20[] = {
    0x00, 0xF0, 0x03, 0x00, 0xF0, 0x03, 0xFF, 0x3F, 0x0F, 0xFF, 0x3F, 0x0F,
    0x00, 0xF0, 0x03, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00,
    0xFC, 0x00, 0x00, 0xCF, 0xFF, 0x0F, 0xCF, 0xFF, 0x0F, 0xFC, 0x00, 0x00,
    0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x03, 0x00, 0xF0, 0x03,
    0xFF, 0x3F, 0x0F, 0xFF, 0x3F, 0x0F, 0x00, 0xF0, 0x03, 0x00, 0xF0, 0x03,
};

const PROGMEM uint8_t battery_charging_20x7[] = {
    0x00, 0xff, 0x03, 0x84, 0x00, 0x04, 0x82, 0x02, 0x0c, 0x8f, 0x02,
    0x0c, 0x84, 0x02, 0x0c, 0x82, 0x00, 0x04, 0x00, 0xff, 0x03,
};

const PROGMEM uint8_t notifications_20x20[] = {
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x00, 0x80, 0x1f, 0x00,
    0xc0, 0x3f, 0x00, 0xe0, 0x70, 0x00, 0x70, 0xe0, 0x00, 0x30, 0xc0, 0x00,
    0x30, 0xc0, 0x00, 0x30, 0xc0, 0x00, 0x30, 0xc0, 0x00, 0x38, 0xc0, 0x01,
    0x1c, 0x80, 0x03, 0xfe, 0x03, 0x07, 0x06, 0x00, 0x06, 0x06, 0x00, 0x06,
    0xfe, 0xff, 0x07, 0xfe, 0xff, 0x07, 0x00, 0x70, 0x00, 0x00, 0x60, 0x00,
};
const PROGMEM uint8_t alarm_20x20[] = {
    0x00, 0x00, 0x00, 0x18, 0x80, 0x01, 0x1c, 0x80, 0x03, 0xce, 0x3f, 0x07,
    0xe7, 0x7f, 0x0e, 0x73, 0xe0, 0x0c, 0x38, 0xc6, 0x01, 0x1c, 0x86, 0x03,
    0x0c, 0x06, 0x03, 0x0c, 0x06, 0x03, 0x0c, 0x7e, 0x03, 0x0c, 0x7e, 0x03,
    0x0c, 0x00, 0x03, 0x0c, 0x00, 0x03, 0x1c, 0x80, 0x01, 0x38, 0xc0, 0x01,
    0x70, 0xe0, 0x00, 0xe0, 0x7f, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0x00, 0x00,
};
