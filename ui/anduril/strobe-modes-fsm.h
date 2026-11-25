// strobe-modes-fsm.h: FSM config for strobe modes in Anduril.
// Copyright (C) 2017-2023 Selene ToyKeeper
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// enable the random number generator if we need it
#if defined(USE_LIGHTNING_MODE) || defined(USE_CANDLE_MODE)
#define USE_PSEUDO_RAND
#endif

// party strobe uses really short pulses
#ifdef USE_PARTY_STROBE_MODE
#define USE_DELAY_ZERO
#endif

// candle mode is basically a bunch of stacked random triangle waves
#if defined(USE_CANDLE_MODE)
#define USE_TRIANGLE_WAVE
#endif

// the presence of strobe mode(s) affects how many eeprom bytes we need,
// so it's relevant for FSM configuration
#if defined(USE_CANDLE_MODE) || defined(USE_BIKE_FLASHER_MODE) || defined(USE_PARTY_STROBE_MODE) || defined(USE_TACTICAL_STROBE_MODE) || defined(USE_LIGHTNING_MODE)
#define USE_STROBE_STATE
#endif

// police strobe brightness can be adjusted with 1H/2H,
// if the light can dim its police colors
// (main LEDs, or aux LEDs which follow the ramp)
// (not on lights with off/low/high-only aux LEDs, since it would do nothing)
#if defined(USE_POLICE_COLOR_STROBE_MODE) && defined(USE_STROBE_STATE) \
    && ((!defined(POLICE_STROBE_USES_AUX)) \
        || defined(USE_AUXRGB_RAMPED_CHANNELS))
#define USE_POLICE_STROBE_BRIGHTNESS
#endif

// internal numbering for strobe modes
#ifdef USE_STROBE_STATE
typedef enum {
    #ifdef USE_PARTY_STROBE_MODE
    party_strobe_e,
    #endif
    #ifdef USE_TACTICAL_STROBE_MODE
    tactical_strobe_e,
    #endif
    #ifdef USE_POLICE_COLOR_STROBE_MODE
    police_color_strobe_e,
    #endif
    #ifdef USE_LIGHTNING_MODE
    lightning_storm_e,
    #endif
    #ifdef USE_CANDLE_MODE
    candle_mode_e,
    #endif
    #ifdef USE_BIKE_FLASHER_MODE
    bike_flasher_e,
    #endif
    strobe_mode_END
} strobe_mode_te;

//const int NUM_STROBES = strobe_mode_END;
#define NUM_STROBES strobe_mode_END
#endif

