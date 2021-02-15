// Copyright © 2021 Cosku Bas. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE.md file.

#include "device.h"
#include "debug.h"

#include <EEPROM.h>
#include <MIDIUSB.h>

Device::Device(const deviceConfig_t* cfg) {
    uint8_t status = fetchByte(ADDR_STATUS);
    switch (status) {
    case CONFIG_CUSTOM:
        // Custom config
        intensity      = fetchByte(ADDR_INTENSITY);
        noteChannel    = fetchByte(ADDR_BASE_CHANNEL);
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
        storeByte(ADDR_INTENSITY, intensity);
        for (int i = 0; i < DEV_MAX_BTN; i++) {
            storeButtonConfig(i, btnConfig[i]);
        }
        DEBUG_PRINTF("Default config has been written to EEPROM\n");
    };

    DEBUG_PRINTF("Status: %d\n", status);
    DEBUG_PRINTF("Note channel: %d\n", noteChannel);
    DEBUG_PRINTF("Control channel: %d\n", controlChannel);
    DEBUG_PRINTF("Intensity: %d\n", intensity);

    // Initiate button states
    for (int i = 0; i < DEV_MAX_BTN; i++) {
        btnState[i] = new ezButton(cfg->btnPins[i]);
        btnState[i]->setDebounceTime(DEV_BTN_DEBOUNCE);
    }

    // Initiate LEDs
    ledState = new MD_MAX72XX(MD_MAX72XX::GENERIC_HW, cfg->mosiPin, cfg->sckPin, cfg->csPin, 1);
    ledState->begin();
    ledState->control(MD_MAX72XX::INTENSITY, intensity);
    ledState->control(MD_MAX72XX::SCANLIMIT, DEV_BTN_COLUMNS - 1);
    for (int i = 0; i < DEV_MAX_BTN; i++) {
        setButtonLed(i, btnConfig[i].released);
    }
};

Device::~Device() {
    // Clean LEDs
    ledState->clear();
    delete ledState;
    ledState = NULL;

    // Clean buttons
    for (int i = 0; i < DEV_MAX_BTN; i++) {
        delete btnState[i];
        btnState[i] = NULL;
    }
}

void Device::UpdateState() {
    // Main loop
    for (int i = 0; i < DEV_MAX_BTN; i++) {
        // Update button states
        ezButton* b = btnState[i];
        b->loop();

        // Process pressed
        if (b->isPressed()) {
            setButtonLed(i, btnConfig[i].pressed);
            if (btnConfig[i].momentary) {
                midiControlOn(i);
            }
            midiNoteOn(i);
        }

        // Process released
        if (b->isReleased()) {
            setButtonLed(i, btnConfig[i].released);
            midiNoteOff(i);
            if (btnConfig[i].momentary) {
                midiControlOff(i);
            }
        }
    }

    // We only check once for programming mode
    if (once) {
        DEBUG_PRINTF("Checking for programming mode\n");

        once             = false;
        bool programming = true;

        for (int i = 0; i < DEV_MAX_BTN; i++) {
            // 3 corner buttons needs to be in pressed state
            // to activate programming mode during boot
            //
            //   * . . *
            //   . . . .
            //   . . . .
            //   . . . *
            //
            if (i == 0 || i == 3 || i == 15) {
                programming = programming && !btnState[i]->getState();
            } else {
                programming = programming && btnState[i]->getState();
            }

            DEBUG_PRINTF("Button %d state is: %d\n", i + 1, btnState[i]->getState());
        }

        if (programming) {
            runProgrammer();
        } else {
            DEBUG_PRINTF("Programming mode skipped\n");
        }
    }
}

void Device::runProgrammer() {
    DEBUG_PRINTF("Programming mode activated\n");

#ifndef MIDICADE_DEBUG
    Serial.begin(115200);
    while (!Serial) { };
#endif

    // Clean all buffers
    Serial.flush();
    while (Serial.read() != -1) { }

    // Commands
    bool running = true;
    while (running) {
        uint8_t buffer[3] = { 0, 0, 0 };
        Serial.readBytes(buffer, sizeof(buffer));

        switch (buffer[0]) {
        case PRG_DONE:
            running = false;
            break;
        case PRG_ECHO:
            Serial.write(PRG_ECHO);
            break;
        case PRG_FETCH:
            Serial.write(noteChannel);
            Serial.write(intensity);
            for (int i = 0; i < DEV_MAX_BTN; i++) {
                Serial.write(btnConfig[i].note);
                Serial.write(btnConfig[i].velocity);
                Serial.write(btnConfig[i].pressed);
                Serial.write(btnConfig[i].released);
                Serial.write(btnConfig[i].momentary);
            }
            break;
        case SET_STATUS:
            if (buffer[1] == CONFIG_DEFAULT || buffer[1] == CONFIG_CUSTOM || buffer[1] == CONFIG_RESET) {
                storeByte(ADDR_STATUS, buffer[1]);
                DEBUG_PRINTF("Status has been set to: %d\n", buffer[1]);
            }
            break;
        case SET_BASE_CHANNEL:
            if (buffer[1] >= 0 && buffer[1] < MIDI_MAX_CHANNEL) {
                noteChannel    = buffer[1];
                controlChannel = noteChannel + 1;
                storeByte(ADDR_BASE_CHANNEL, noteChannel);
                DEBUG_PRINTF("Base channel has been set to: %d\n", noteChannel);
            }
            break;
        case SET_INTENSITY:
            if (buffer[1] >= 0 && buffer[1] <= MAX_INTENSITY) {
                intensity = buffer[1];
                storeByte(ADDR_INTENSITY, intensity);
                DEBUG_PRINTF("Intensity has been set to: %d\n", intensity);
            }
            break;
        case SET_PRESSED_LED:
            if (buffer[1] == LED_COLOR_1 || buffer[1] == LED_COLOR_2 || buffer[1] == LED_OFF) {
                if (buffer[2] >= 0 && buffer[2] < DEV_MAX_BTN) {
                    btnConfig[buffer[2]].pressed = ledValue_t(buffer[1]);
                    storeButtonConfig(buffer[2], btnConfig[buffer[2]]);
                    DEBUG_PRINTF("Button %d pressed LED has been set to: %d\n", buffer[2], buffer[1]);
                }
            }
            break;
        case SET_RELEASED_LED:
            if (buffer[1] == LED_COLOR_1 || buffer[1] == LED_COLOR_2 || buffer[1] == LED_OFF) {
                if (buffer[2] >= 0 && buffer[2] < DEV_MAX_BTN) {
                    btnConfig[buffer[2]].released = ledValue_t(buffer[1]);
                    storeButtonConfig(buffer[2], btnConfig[buffer[2]]);
                    DEBUG_PRINTF("Button %d released LED has been set to: %d\n", buffer[2], buffer[1]);
                }
            }
            break;
        case SET_MIDI_NOTE:
            if (buffer[1] >= 0 && buffer[1] <= MIDI_MAX_NOTE) {
                if (buffer[2] >= 0 && buffer[2] < DEV_MAX_BTN) {
                    btnConfig[buffer[2]].note = buffer[1];
                    storeButtonConfig(buffer[2], btnConfig[buffer[2]]);
                    DEBUG_PRINTF("Button %d MIDI note has been set to: %d\n", buffer[2], buffer[1]);
                }
            }
            break;
        case SET_MIDI_VELOCITY:
            if (buffer[1] >= 0 && buffer[1] <= MIDI_MAX_VELOCITY) {
                if (buffer[2] >= 0 && buffer[2] < DEV_MAX_BTN) {
                    btnConfig[buffer[2]].velocity = buffer[1];
                    storeButtonConfig(buffer[2], btnConfig[buffer[2]]);
                    DEBUG_PRINTF(
                        "Button %d MIDI velocity has been set to: %d\n", buffer[2], buffer[1]);
                }
            }
            break;
        case SET_MOMENTARY:
            if (buffer[1] == 0 || buffer[1] == 1) {
                if (buffer[2] >= 0 && buffer[2] < DEV_MAX_BTN) {
                    btnConfig[buffer[2]].momentary = buffer[1];
                    storeButtonConfig(buffer[2], btnConfig[buffer[2]]);
                    DEBUG_PRINTF("Button %d CC flag has been set to: %d\n", buffer[2], buffer[1]);
                }
            }
            break;
        }
    }

#ifndef MIDICADE_DEBUG
    Serial.end();
#endif

    DEBUG_PRINTF("Programming mode terminated\n");
}

void Device::storeButtonConfig(uint8_t button, btnConfig_t config) {
    uint16_t offset = ADDR_CONFIG + button * sizeof(btnConfig_t);
    EEPROM.put(offset, config);
}

btnConfig_t Device::fetchButtonConfig(uint8_t button) {
    uint16_t    offset = ADDR_CONFIG + button * sizeof(btnConfig_t);
    btnConfig_t config;
    EEPROM.get(offset, config);
    return config;
}

void Device::storeByte(uint16_t addr, uint8_t data) {
    EEPROM.put(addr, data);
}

uint8_t Device::fetchByte(uint16_t addr) {
    uint8_t data;
    EEPROM.get(addr, data);
    return data;
}

void Device::setButtonLed(uint8_t button, ledValue_t value) {
    int rowa   = button / DEV_BTN_COLUMNS;
    int rowb   = rowa + DEV_BTN_ROWS;
    int column = button % DEV_BTN_COLUMNS;

    switch (value) {
    case LED_OFF:
        ledState->setPoint(rowa, column, false);
        ledState->setPoint(rowb, column, false);
        break;
    case LED_COLOR_1:
        ledState->setPoint(rowa, column, true);
        ledState->setPoint(rowb, column, false);
        break;
    default:
        ledState->setPoint(rowa, column, false);
        ledState->setPoint(rowb, column, true);
    }

    DEBUG_PRINTF("Button %d LED has been set to: %d\n", button + 1, value);
}

void Device::midiNoteOn(uint8_t button) {
    midiEventPacket_t noteOn = {
        .header = 0x09,
        .byte1  = 0x90 | noteChannel,
        .byte2  = btnConfig[button].note,
        .byte3  = btnConfig[button].velocity,
    };
    MidiUSB.sendMIDI(noteOn);
    MidiUSB.flush();

    DEBUG_PRINTF("MIDI noteOn: %d %d %d %d\n", noteOn.header, noteOn.byte1, noteOn.byte2, noteOn.byte3);
}

void Device::midiNoteOff(uint8_t button) {
    midiEventPacket_t noteOff = {
        .header = 0x08,
        .byte1  = 0x80 | noteChannel,
        .byte2  = btnConfig[button].note,
        .byte3  = btnConfig[button].velocity,
    };
    MidiUSB.sendMIDI(noteOff);
    MidiUSB.flush();

    DEBUG_PRINTF("MIDI noteOff: %d %d %d %d\n", noteOff.header, noteOff.byte1, noteOff.byte2, noteOff.byte3);
}

void Device::midiControlOn(uint8_t button) {
    midiEventPacket_t controlOn = {
        .header = 0x0B,
        .byte1  = 0xB0 | controlChannel,
        .byte2  = btnConfig[button].note,
        .byte3  = 0x7F,
    };
    MidiUSB.sendMIDI(controlOn);
    MidiUSB.flush();

    DEBUG_PRINTF("MIDI controlOn: %d %d %d %d\n", controlOn.header, controlOn.byte1, controlOn.byte2, controlOn.byte3);
}

void Device::midiControlOff(uint8_t button) {
    midiEventPacket_t controlOff = {
        .header = 0x0B,
        .byte1  = 0xB0 | controlChannel,
        .byte2  = btnConfig[button].note,
        .byte3  = 0x00,
    };
    MidiUSB.sendMIDI(controlOff);
    MidiUSB.flush();

    DEBUG_PRINTF("MIDI controlOff: %d %d %d %d\n", controlOff.header, controlOff.byte1, controlOff.byte2, controlOff.byte3);
}