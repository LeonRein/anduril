// channel modes for RGB aux LEDs
// Copyright (C) 2023-2026 Selene ToyKeeper
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#ifdef USE_AUXRGB_RAMPED_CHANNELS

// color of each aux channel mode, starting at CM_AUXRED
// (bits: 0b00000BGR)
PROGMEM const uint8_t auxrgb_channel_colors[] = {
    0b001,  // red
    0b011,  // yellow
    0b010,  // green
    0b110,  // cyan
    0b100,  // blue
    0b101,  // purple
    0b111,  // white
};

// hwdef: convert a 1-based aux ramp level to a raw RGB brightness value
rgb_uint_t get_level_auxrgb(uint8_t level);

// aux color channels (and HSV) at a brightness from the aux RGB ramp
void set_level_auxrgb_ramped(uint8_t level) {
    // level is 0-based here, but get_level_auxrgb() is 1-based
    rgb_uint_t v = get_level_auxrgb(level + 1);
    RGB_t color;

    #if defined(USE_HSV2RGB) && defined(USE_CHANNEL_MODE_ARGS)
    if (channel_has_args(channel_mode)) {  // HSV channel, hue = mode arg
        color = hsv2rgb(cfg.channel_mode_args[channel_mode], 255, v);
    } else
    #endif
    {  // fixed color
        uint8_t mask = pgm_read_byte(auxrgb_channel_colors
                                     + (channel_mode - CM_AUXRED));
        color.r = (mask & 0b001) ? v : 0;
        color.g = (mask & 0b010) ? v : 0;
        color.b = (mask & 0b100) ? v : 0;
    }

    set_auxrgb_pwm(color);
}

#else  // passive aux color channels (off / low / high only)

void set_level_auxred(uint8_t level) {
    set_auxrgb_power(0b000001 << !(!(level)));  // red, high (level > 0) or low
}

void set_level_auxyel(uint8_t level) {
    set_auxrgb_power(0b000101 << !(!(level)));  // red+green, high (level > 0) or low
}

void set_level_auxgrn(uint8_t level) {
    set_auxrgb_power(0b000100 << !(!(level)));  // green, high (level > 0) or low
}

void set_level_auxcyn(uint8_t level) {
    set_auxrgb_power(0b010100 << !(!(level)));  // green+blue, high (level > 0) or low
}

void set_level_auxblu(uint8_t level) {
    set_auxrgb_power(0b010000 << !(!(level)));  // blue, high (level > 0) or low
}

void set_level_auxprp(uint8_t level) {
    set_auxrgb_power(0b010001 << !(!(level)));  // red+blue, high (level > 0) or low
}

void set_level_auxwht(uint8_t level) {
    set_auxrgb_power(0b010101 << !(!(level)));  // red+green+blue, high (level > 0) or low
}

#endif  // ifdef USE_AUXRGB_RAMPED_CHANNELS

bool gradual_tick_null(uint8_t gt) { return true; }  // do nothing

