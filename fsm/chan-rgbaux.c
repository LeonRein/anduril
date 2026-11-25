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
// (declared here because rgb_uint_t isn't defined yet in chan-rgbaux.h)
rgb_uint_t get_level_auxrgb(uint8_t level);

// the UI can make aux channels reach full brightness at a lower level
// (like the ramp ceiling, since aux LEDs have no turbo)
#ifndef AUXRGB_RAMP_TOP
#define AUXRGB_RAMP_TOP  MAX_LEVEL
#endif

// brightness for a 0-based channel level, scaled so the top level
// (AUXRGB_RAMP_TOP) and anything above it is full aux brightness
rgb_uint_t auxrgb_ramped_brightness(uint8_t level) {
    uint8_t top = AUXRGB_RAMP_TOP;
    level ++;  // get_level_auxrgb() is 1-based
    if (level >= top) level = MAX_LEVEL;
    else level = (uint16_t)level * MAX_LEVEL / top;
    return get_level_auxrgb(level);
}

// solid aux colors, at a brightness from the aux RGB ramp
void set_level_auxrgb_color(uint8_t level) {
    rgb_uint_t v = auxrgb_ramped_brightness(level);
    uint8_t mask = pgm_read_byte(auxrgb_channel_colors
                                 + (channel_mode - CM_AUXRED));
    RGB_t color;
    color.r = (mask & 0b001) ? v : 0;
    color.g = (mask & 0b010) ? v : 0;
    color.b = (mask & 0b100) ? v : 0;
    set_auxrgb_pwm(color);
}

#if defined(USE_HSV2RGB) && defined(USE_CHANNEL_MODE_ARGS)
// adjustable hue (channel mode arg), at a brightness from the aux RGB ramp
void set_level_auxrgb_hsv(uint8_t level) {
    #ifdef USE_CFG
    uint8_t h = cfg.channel_mode_args[channel_mode];
    #else
    uint8_t h = channel_mode_args[channel_mode];
    #endif
    rgb_uint_t v = auxrgb_ramped_brightness(level);
    set_auxrgb_pwm(hsv2rgb(h, 255, v));
}
#endif

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

