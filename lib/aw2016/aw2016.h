// aw2016.h: Device driver for AW2016 peripheral
// Copyright (C) 2025 - 2026 Loneoceans
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>  // for PROGMEM


////////// defs and types //////////

// if you #include this file, assume you're using this chip and this code
#define USE_AW2016

// rgb channel numbers (different per build target)
#define AW_R_CH  1
#define AW_G_CH  0
#define AW_B_CH  2

// active (PWM) modes use 16-bit "aw2016 units", see AW2016_UNITS_MAX below
#define RGB_BITS  16

// The chip is used in a few different ways:
// - Passive (aw2016_set_auxrgb_power): off / low / high per color, using the
//   user's standby "low" and "high" rows from aw2016_ramp[].
// - Animations (aw2016_animation_do): breathing patterns run by the chip,
//   also using the standby rows.
// - Active, auto-ranging (aw2016_set_auxrgb_pwm): any color and brightness,
//   in aw2016 units.  Used by aux channel modes, which follow the ramp.
// - Active, fixed row (aw2016_set_auxrgb_pwm_row): raw PWM on top of one row
//   of aw2016_ramp[].  Used by POVD / batt color mode, where the user picks
//   a row, so the steady display uses analog dimming instead of PWM.

// available aux RGB hardware power levels
// (power used at PWM=255)
typedef enum {
    AW2016_5mA  = 0b10,
    AW2016_10mA = 0b11,
    AW2016_15mA = 0b00,
    AW2016_30mA = 0b01,
} aw2016_current_t;


////////// const data //////////

// coarse brightness ramp and color balance using as little PWM as possible
// PWM modes are hella strobey, but it's unavoidable
// (power levels are estimated, not actual, and don't include overhead)
// FIXME: green is still too bright sometimes
PROGMEM const uint8_t aw2016_ramp[] = {
    // power   ,  R, G, B, PWM
    AW2016_5mA ,  4, 1, 1,   2,  // barely visible
    AW2016_5mA ,  4, 1, 1,   6,  // 0.03 mA
    AW2016_5mA ,  4, 1, 1,  24,  // 0.13 mA  <-- about right for "aux low" mode
    AW2016_5mA ,  4, 1, 1,  75,  // 0.4 mA
    AW2016_5mA ,  4, 1, 1, 150,  // 0.8 mA
    AW2016_5mA ,  4, 1, 1, 255,  // 1.3 mA
    AW2016_5mA ,  9, 2, 3, 255,  //   3 mA  <-- default POVD
    AW2016_5mA , 12, 3, 4, 255,  //   4 mA
    AW2016_5mA , 15, 4, 5, 255,  //   5 mA  <-- about right for "aux high" mode
    AW2016_10mA, 12, 3, 4, 255,  //   8 mA
    AW2016_10mA, 15, 4, 5, 255,  //  10 mA
    AW2016_15mA, 15, 4, 5, 255,  //  15 mA
    AW2016_30mA, 12, 3, 4, 255,  //  24 mA
    AW2016_30mA, 15, 4, 5, 255,  //  30 mA  <-- "R G Bling" modes
    // duplicate, to make it easier to hit the top level
    // (requested by Hank)
    AW2016_30mA, 15, 4, 5, 255,  //  dupe
};
#define aw2016_ramp_get(row,col)  pgm_read_byte(aw2016_ramp + (5*row) + col)
#define AW2016_RAMP_SIZE    (sizeof(aw2016_ramp))
#define AW2016_RAMP_LEVELS  (sizeof(aw2016_ramp)/5)
#define AW2016_RAMP_DEFAULT    8
#define AW2016_RAMP_MIN_NOPWM  5
// default for each configurable brightness
// (0-based row indexes into table above)
#define AW2016_DEFAULT_LO    2
#define AW2016_DEFAULT_HI    8
#define AW2016_DEFAULT_POVD  6

// Active (PWM) modes use automatic power level selection ("auto-ranging").
// Brightness is given in "aw2016 units", per color channel:
//   1 unit = PWM 1/255 at 5 mA, with the color balance below
// The driver picks the lowest current which can show the requested color,
// then scales the PWM values to match.  So the same brightness always looks
// the same, regardless of which current range is used, and dim colors use
// the highest PWM duty cycle possible (less flicker, finer color steps).
// Current ranges are 5, 10, 15, 30 mA, i.e. multipliers 1, 2, 3, 6.
#define AW2016_RANGE_MULT_MAX  6  // 30 mA / 5 mA
#define AW2016_UNITS_MAX  (255 * AW2016_RANGE_MULT_MAX)  // 30 mA at PWM 255
// how many units make 1 PWM step at the highest current range
#define AW2016_UNITS_PER_PWM  AW2016_RANGE_MULT_MAX
// convert aw2016 units to 0..255 PWM for fixed-row mode (rounded up,
// so a non-zero brightness stays non-zero)
#define aw2016_units_to_row_pwm(u)  \
    (((u) + (AW2016_UNITS_PER_PWM - 1)) / AW2016_UNITS_PER_PWM)

// how active PWM mode is configured (aw2016_active_range):
//   0..3 = auto-ranging, index into aw2016_range_current[]
//   AW2016_RANGE_ROW | row = fixed row from aw2016_ramp[]
//   AW2016_RANGE_NONE = not configured yet
#define AW2016_RANGE_ROW   0x80
#define AW2016_RANGE_NONE  0xff
// color balance for active modes (same as the brightest standby rows)
#ifndef AW2016_ACTIVE_CAL_R
#define AW2016_ACTIVE_CAL_R  15
#endif
#ifndef AW2016_ACTIVE_CAL_G
#define AW2016_ACTIVE_CAL_G   4
#endif
#ifndef AW2016_ACTIVE_CAL_B
#define AW2016_ACTIVE_CAL_B   5
#endif

//typedef enum {
//    AW2016_MODE_PASSIVE,
//    AW2016_MODE_ACTIVE,
//    AW2016_MODE_POVD,
//    AW2016_MODE_ANIM,
//} aw2016_mode_t;


////////// globals //////////

uint8_t aw2016_is_awake = 0;
uint8_t aw2016_is_pwm_mode = 0;
uint8_t aw2016_animation_running = 0;
uint8_t aw2016_animation_color = 0;
// allow sleep by default, set to 1 if fast response required from driver
uint8_t aw2016_do_not_sleep = 0;
// how active PWM mode is configured (see AW2016_RANGE_* above)
uint8_t aw2016_active_range = AW2016_RANGE_NONE;


////////// public API functions //////////

// call once at boot, from hwdef_setup()
inline void aw2016_hwdef_setup ();
// reset to default state
void aw2016_init ();

// activate a row in the aw2016 brightness ramp
void aw2016_set_ramp_current (uint8_t row);
// helper for rotating through ramp levels
uint8_t aw2016_next_ramp_row (uint8_t row, bool nopwm);

// set raw PWM values (0 to 255, not aw2016 units) on top of a fixed row of
// aw2016_ramp[] (no auto-ranging)
void aw2016_set_auxrgb_pwm_row (RGB16_t pwm, uint8_t row);

// start or continue an animation
void aw2016_animation_do (uint8_t pattern, uint8_t color);

// pre-built effects
void aw2016_breathe (uint8_t color, uint8_t speed, uint8_t pwm);
void aw2016_rainbow_rgb ();
void aw2016_rainbow_blend ();


////////// common auxrgb API stuff //////////

// emulate passive auxrgb
#undef set_auxrgb_power
#define set_auxrgb_power  aw2016_set_auxrgb_power
void aw2016_set_auxrgb_power (uint8_t power);

// change to active PWM mode or "passive" off/low/high mode
#define enable_auxrgb_pwm  aw2016_enable_auxrgb_pwm
#define disable_auxrgb_pwm  aw2016_disable_auxrgb_pwm
void aw2016_enable_auxrgb_pwm ();
void aw2016_disable_auxrgb_pwm ();

// set RGB values for active PWM mode
// (in aw2016 units, 0 to AW2016_UNITS_MAX per channel, auto-ranging)
#define set_auxrgb_pwm  aw2016_set_auxrgb_pwm
void aw2016_set_auxrgb_pwm (RGB16_t color);

