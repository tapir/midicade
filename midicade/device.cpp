// Copyright © 2021 Cosku Bas. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE.md file.

#include "device.h"
#include "debug.h"

#include <MIDIUSB.h>

Device::Device() {
    uint8_t status = fetchByte(ADDR_STATUS);
    switch (status) {
    case CONFIG_CUSTOM:
        // Custom config
        noteChannel = fetchByte(ADDR_BASE_CHANNEL);
        controlChannel = noteChannel + 1;
        for (int i = 0; i < DEV_MAX_BTN; i++) {
            btnConfig[i] = fetchButtonConfig(i);
        }
        DEBUG_PRINTF("Custom config has been read from EEPROM\n");
        break;
    case CONFIG_DEFAULT:
        // Default config
        DEBUG_PRINTF("Default config is being used\n");
        break;
    default:
        // Anything else needs to be updated with the default config
        storeByte(ADDR_STATUS, CONFIG_DEFAULT);
        storeByte(ADDR_BASE_CHANNEL, noteChannel);
        for (int i = 0; i < DEV_MAX_BTN; i++) {
            storeButtonConfig(i, btnConfig[i]);
        }
        DEBUG_PRINTF("Default config has been written to EEPROM\n");
    };

    DEBUG_PRINTF("Status: %d\n", status);
    DEBUG_PRINTF("Note channel: %d\n", noteChannel);
    DEBUG_PRINTF("Control channel: %d\n", controlChannel);

    // Initiate button states
    btnStates[0] = new ezButton(PIN_BUTTON_1);
    btnStates[0]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[1] = new ezButton(PIN_BUTTON_2);
    btnStates[1]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[2] = new ezButton(PIN_BUTTON_3);
    btnStates[2]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[3] = new ezButton(PIN_BUTTON_4);
    btnStates[3]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[4] = new ezButton(PIN_BUTTON_5);
    btnStates[4]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[5] = new ezButton(PIN_BUTTON_6);
    btnStates[5]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[6] = new ezButton(PIN_BUTTON_7);
    btnStates[6]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[7] = new ezButton(PIN_BUTTON_8);
    btnStates[7]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[8] = new ezButton(PIN_BUTTON_9);
    btnStates[8]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[9] = new ezButton(PIN_BUTTON_10);
    btnStates[9]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[10] = new ezButton(PIN_BUTTON_11);
    btnStates[10]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[11] = new ezButton(PIN_BUTTON_12);
    btnStates[11]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[12] = new ezButton(PIN_BUTTON_13);
    btnStates[12]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[13] = new ezButton(PIN_BUTTON_14);
    btnStates[13]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[14] = new ezButton(PIN_BUTTON_15);
    btnStates[14]->setDebounceTime(DEV_BTN_DEBOUNCE);
    btnStates[15] = new ezButton(PIN_BUTTON_16);
    btnStates[15]->setDebounceTime(DEV_BTN_DEBOUNCE);

    // Initiate LEDs
    FastLED.addLeds<WS2812B, PIN_LED_ARRAY_1, GRB>(ledArrays[0], DEV_LEDS_PER_ARRAY);
    FastLED.addLeds<WS2812B, PIN_LED_ARRAY_2, GRB>(ledArrays[1], DEV_LEDS_PER_ARRAY);
    FastLED.addLeds<WS2812B, PIN_LED_ARRAY_3, GRB>(ledArrays[2], DEV_LEDS_PER_ARRAY);
    FastLED.addLeds<WS2812B, PIN_LED_ARRAY_4, GRB>(ledArrays[3], DEV_LEDS_PER_ARRAY);
    for (int i = 0; i < DEV_MAX_BTN * 2 - 1; i += 2) {
        int button = i / 2;
        ledStates[button] = &(ledArrays[ledPinMap[i]][ledPinMap[i + 1]]);
        setButtonLedReleased(button);
    }
};

Device::~Device() {
    // Clean buttons
    for (int i = 0; i < DEV_MAX_BTN; i++) {
        delete btnStates[i];
        btnStates[i] = NULL;
    }
}

void Device::UpdateState() {
    // Main loop
    for (int i = 0; i < DEV_MAX_BTN; i++) {
        // Update button states
        ezButton *b = btnStates[i];
        b->loop();

        // Process pressed
        if (b->isPressed()) {
            setButtonLedPressed(i);
            midiNoteOn(i);
            if (btnConfig[i].momentary) {
                midiControlOn(i);
            }
        }

        // Process released
        if (b->isReleased()) {
            setButtonLedReleased(i);
            midiNoteOff(i);
            if (btnConfig[i].momentary) {
                midiControlOff(i);
            }
        }
    }

    // We only check once for programming mode, right after power up
    if (once) {
        once = false;

        // 3 corner buttons needs to be in pressed state
        // to activate programming mode during boot (Buttons 1, 4, 16)
        if (!btnStates[0]->getState() && !btnStates[3]->getState() && !btnStates[15]->getState()) {
            runProgrammer();
        }
    }
}

void Device::setButtonLedPressed(uint8_t button) {
    CRGB color = btnConfig[button].colorPressed;
    *(ledStates[button]) = color;
    FastLED.show();
    DEBUG_PRINTF("Button %d LED color has been set to pressed: 0x%02x%02x%02x\n", button + 1, color[0], color[1], color[2]);
}

void Device::setButtonLedReleased(uint8_t button) {
    CRGB color = btnConfig[button].colorReleased;
    *(ledStates[button]) = color;
    FastLED.show();
    DEBUG_PRINTF("Button %d LED color has been set to released: 0x%02x%02x%02x\n", button + 1, color[0], color[1], color[2]);
}

void Device::setButtonLedColor(uint8_t button, CRGB color) {
    *(ledStates[button]) = color;
    FastLED.show();
    DEBUG_PRINTF("Button %d LED color has been set to: 0x%02x%02x%02x\n", button + 1, color[0], color[1], color[2]);
}

void Device::midiNoteOn(uint8_t button) {
    midiEventPacket_t noteOn = {
        .header = 0x09,
        .byte1 = 0x90 | noteChannel,
        .byte2 = btnConfig[button].midiNote,
        .byte3 = btnConfig[button].midiVelocity,
    };
    MidiUSB.sendMIDI(noteOn);
    MidiUSB.flush();
    DEBUG_PRINTF("MIDI noteOn: %d %d %d %d\n", noteOn.header, noteOn.byte1, noteOn.byte2, noteOn.byte3);
}

void Device::midiNoteOff(uint8_t button) {
    midiEventPacket_t noteOff = {
        .header = 0x08,
        .byte1 = 0x80 | noteChannel,
        .byte2 = btnConfig[button].midiNote,
        .byte3 = btnConfig[button].midiVelocity,
    };
    MidiUSB.sendMIDI(noteOff);
    MidiUSB.flush();
    DEBUG_PRINTF("MIDI noteOff: %d %d %d %d\n", noteOff.header, noteOff.byte1, noteOff.byte2, noteOff.byte3);
}

void Device::midiControlOn(uint8_t button) {
    midiEventPacket_t controlOn = {
        .header = 0x0B,
        .byte1 = 0xB0 | controlChannel,
        .byte2 = btnConfig[button].midiNote,
        .byte3 = 0x7F,
    };
    MidiUSB.sendMIDI(controlOn);
    MidiUSB.flush();
    DEBUG_PRINTF("MIDI controlOn: %d %d %d %d\n", controlOn.header, controlOn.byte1, controlOn.byte2, controlOn.byte3);
}

void Device::midiControlOff(uint8_t button) {
    midiEventPacket_t controlOff = {
        .header = 0x0B,
        .byte1 = 0xB0 | controlChannel,
        .byte2 = btnConfig[button].midiNote,
        .byte3 = 0x00,
    };
    MidiUSB.sendMIDI(controlOff);
    MidiUSB.flush();
    DEBUG_PRINTF("MIDI controlOff: %d %d %d %d\n", controlOff.header, controlOff.byte1, controlOff.byte2, controlOff.byte3);
}