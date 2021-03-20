// Copyright © 2021 Cosku Bas. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE.md file.

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <FastLED.h>
#include <ezButton.h>
#include <stdint.h>

// Status codes
#define CONFIG_DEFAULT 0xFD
#define CONFIG_CUSTOM  0xFE
#define CONFIG_RESET   0xFF

// EEPROM address offsets
#define ADDR_STATUS       0x00
#define ADDR_BASE_CHANNEL 0x01
#define ADDR_CONFIG       0x02

// Device parameters
#define DEV_BTN_DEBOUNCE   3 // miliseconds
#define DEV_MAX_BTN        16
#define DEV_LEDS_PER_ARRAY 4
#define DEV_MAX_LED_ARRAYS 4

// MIDI
#define MIDI_CHANNEL_BASE 2
#define MIDI_MAX_CHANNEL  16
#define MIDI_MAX_NOTE     127
#define MIDI_MAX_VELOCITY 127

// PINS
#define PIN_BUTTON_1    22
#define PIN_BUTTON_2    23
#define PIN_BUTTON_3    3
#define PIN_BUTTON_4    2
#define PIN_BUTTON_5    21
#define PIN_BUTTON_6    18
#define PIN_BUTTON_7    1
#define PIN_BUTTON_8    0
#define PIN_BUTTON_9    20
#define PIN_BUTTON_10   19
#define PIN_BUTTON_11   12
#define PIN_BUTTON_12   4
#define PIN_BUTTON_13   13
#define PIN_BUTTON_14   5
#define PIN_BUTTON_15   10
#define PIN_BUTTON_16   6
#define PIN_LED_ARRAY_1 7  // Array 0
#define PIN_LED_ARRAY_2 8  // Array 1
#define PIN_LED_ARRAY_3 9  // Array 2
#define PIN_LED_ARRAY_4 11 // Array 3

const uint8_t ledPinMap[DEV_MAX_BTN * 2] = {
    0, 1, // LED_01 -> Array 0, Index 1
    0, 0, // LED_02 -> Array 0, Index 0
    3, 0, // LED_03 -> Array 3, Index 0
    3, 1, // LED_04 -> Array 3, Index 1
    0, 2, // LED_05 -> Array 0, Index 2
    0, 3, // LED_06 -> Array 0, Index 3
    3, 3, // LED_07 -> Array 3, Index 3
    3, 2, // LED_08 -> Array 3, Index 2
    2, 1, // LED_09 -> Array 2, Index 1
    2, 0, // LED_10 -> Array 2, Index 0
    1, 0, // LED_11 -> Array 1, Index 0
    1, 1, // LED_12 -> Array 1, Index 1
    2, 2, // LED_13 -> Array 2, Index 2
    2, 3, // LED_14 -> Array 2, Index 3
    1, 3, // LED_15 -> Array 1, Index 3
    1, 2, // LED_16 -> Array 1, Index 2
};

// Consts
#define SET_STATUS         0x10
#define SET_BASE_CHANNEL   0x11
#define SET_COLOR_PRESSED  0x12
#define SET_COLOR_RELEASED 0x13
#define SET_MIDI_NOTE      0x14
#define SET_MIDI_VELOCITY  0x15
#define SET_MOMENTARY      0x16
#define PRG_DONE           0x17
#define PRG_ECHO           0x18
#define PRG_FETCH          0x19

class Device {
public:
    // Creates a new Midicade device
    Device();

    // Cleans everything
    ~Device();

    // Processes and updates all state: MIDI, LEDs etc...
    void UpdateState();

private:
    typedef struct btnConfig_t {
        uint8_t midiNote;
        uint8_t midiVelocity;
        CRGB colorPressed;
        CRGB colorReleased;
        bool momentary; // Enable/disable CC messages
    };

    // States
    ezButton *btnStates[DEV_MAX_BTN];
    CRGB *ledStates[DEV_MAX_BTN];
    CRGB ledArrays[DEV_MAX_LED_ARRAYS][DEV_LEDS_PER_ARRAY];

    // Defaults
    uint8_t noteChannel = MIDI_CHANNEL_BASE;
    uint8_t controlChannel = MIDI_CHANNEL_BASE + 1;
    btnConfig_t btnConfig[DEV_MAX_BTN] = {
        { .midiNote = 48, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_01
        { .midiNote = 49, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_02
        { .midiNote = 50, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_03
        { .midiNote = 51, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_04
        { .midiNote = 44, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_05
        { .midiNote = 45, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_06
        { .midiNote = 46, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_07
        { .midiNote = 47, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_08
        { .midiNote = 40, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_09
        { .midiNote = 41, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_10
        { .midiNote = 42, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_11
        { .midiNote = 43, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_12
        { .midiNote = 36, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_13
        { .midiNote = 37, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_14
        { .midiNote = 38, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_15
        { .midiNote = 39, .midiVelocity = 127, .colorPressed = CRGB::Cyan, .colorReleased = CRGB::Black, .momentary = true }, // BUTTON_16
    };

    // Programming mode
    bool once = true;
    void runProgrammer();

    // Changes given button's LED color
    void setButtonLedPressed(uint8_t button);
    void setButtonLedReleased(uint8_t button);
    void setButtonLedColor(uint8_t button, CRGB color);

    // MIDI messages
    void midiNoteOn(uint8_t button);
    void midiNoteOff(uint8_t button);
    void midiControlOn(uint8_t button);
    void midiControlOff(uint8_t button);

    // EEPROM routines
    static void storeButtonConfig(uint8_t button, btnConfig_t config);
    static btnConfig_t fetchButtonConfig(uint8_t button);
    static void storeByte(uint16_t addr, uint8_t data);
    static uint8_t fetchByte(uint16_t addr);
};

#endif