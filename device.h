// Copyright © 2021 Cosku Bas. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE.md file.

#ifndef __DEVICE_H__
#define __DEVICE_H__

// Status codes
#define CONFIG_DEFAULT 0xFD
#define CONFIG_CUSTOM  0xFE
#define CONFIG_RESET   0xFF

// EEPROM addresses
#define ADDR_STATUS       0x00
#define ADDR_BASE_CHANNEL 0x01
#define ADDR_INTENSITY    0x02
#define ADDR_CONFIG       0x03

// Device parameters
#define DEV_BTN_ROWS     4
#define DEV_BTN_COLUMNS  4
#define DEV_BTN_DEBOUNCE 3
#define DEV_MAX_BTN      16
#define DEV_MAX_COLORS   2

// MIDI
#define MIDI_CHANNEL_BASE 2
#define MIDI_MAX_CHANNEL  16
#define MIDI_MAX_NOTE     127
#define MIDI_MAX_VELOCITY 127

// Disable MD_MAX72XX font
#define USE_LOCAL_FONT 0

#include <MD_MAX72xx.h>
#include <ezButton.h>

enum ledValue_t {
    LED_COLOR_1 = 0, // Red
    LED_COLOR_2 = 1, // Blue
    LED_OFF     = 2, // Off
};

typedef struct deviceConfig_t {
    const uint8_t mosiPin; // SPI MOSI
    const uint8_t sckPin;  // SPI SCK
    const uint8_t csPin;   // SPI CS
    const uint8_t btnPins[DEV_MAX_BTN];
};

typedef struct btnConfig_t {
    uint8_t    note;
    uint8_t    velocity;
    ledValue_t pressed;   // Pressed color
    ledValue_t released;  // Released color
    bool       momentary; // Enable/disable CC messages
};

class Device {
public:
    // Creates a new Midicade device with given configuration
    Device(const deviceConfig_t* cfg);

    // Cleans everything
    ~Device();

    // Processes and updates all state: MIDI, LEDs etc...
    void UpdateState();

private:
    // Programming mode serial commands
    enum programmingCmd_t {
        SET_STATUS        = 0x10,
        SET_BASE_CHANNEL  = 0x11,
        SET_INTENSITY     = 0x12,
        SET_PRESSED_LED   = 0x13,
        SET_RELEASED_LED  = 0x14,
        SET_MIDI_NOTE     = 0x15,
        SET_MIDI_VELOCITY = 0x16,
        SET_MOMENTARY     = 0x17,
        PRG_DONE          = 0x18,
        PRG_ECHO          = 0x19,
        PRG_FETCH         = 0x1A,
    };

    // States
    ezButton*   btnState[DEV_MAX_BTN];
    MD_MAX72XX* ledState;

    // Defaults
    uint8_t     intensity              = MAX_INTENSITY;
    uint8_t     noteChannel            = MIDI_CHANNEL_BASE;
    uint8_t     controlChannel         = MIDI_CHANNEL_BASE + 1;
    btnConfig_t btnConfig[DEV_MAX_BTN] = {
        { .note = 48, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_01
        { .note = 49, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_02
        { .note = 50, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_03
        { .note = 51, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_04
        { .note = 44, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_05
        { .note = 45, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_06
        { .note = 46, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_07
        { .note = 47, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_08
        { .note = 40, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_09
        { .note = 41, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_10
        { .note = 42, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_11
        { .note = 43, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_12
        { .note = 36, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_13
        { .note = 37, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_14
        { .note = 38, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_15
        { .note = 39, .velocity = 0x7F, .pressed = LED_OFF, .released = LED_COLOR_1, .momentary = true }, // BUTTON_16
    };

    // Programming mode
    bool once = true;
    void runProgrammer();

    // Changes given button's LED state
    void setButtonLed(uint8_t button, ledValue_t value);

    // MIDI messages
    void midiNoteOn(uint8_t button);
    void midiNoteOff(uint8_t button);
    void midiControlOn(uint8_t button);
    void midiControlOff(uint8_t button);

    // EEPROM routines
    static void        storeButtonConfig(uint8_t button, btnConfig_t config);
    static btnConfig_t fetchButtonConfig(uint8_t button);
    static void        storeByte(uint16_t addr, uint8_t data);
    static uint8_t     fetchByte(uint16_t addr);
};

#endif