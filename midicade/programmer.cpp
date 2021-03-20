#include "debug.h"
#include "device.h"

#include <EEPROM.h>

void Device::runProgrammer() {
    DEBUG_PRINTF("Programming mode activated\n");

#ifndef MIDICADE_DEBUG
    Serial.begin(115200);
    while (!Serial) { };
#endif

    // Clean all serial buffer
    Serial.flush();
    while (Serial.read() != -1) { }

    // Commands
    bool running = true;
    while (running) {
        int button = 0;
        uint8_t buffer[5] = { 0, 0, 0, 0, 0 };

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
            for (int i = 0; i < DEV_MAX_BTN; i++) {
                Serial.write(btnConfig[i].midiNote);
                Serial.write(btnConfig[i].midiVelocity);
                Serial.write(btnConfig[i].colorPressed);
                Serial.write(btnConfig[i].colorReleased);
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
                noteChannel = buffer[1];
                controlChannel = noteChannel + 1;
                storeByte(ADDR_BASE_CHANNEL, noteChannel);
                DEBUG_PRINTF("MIDI base channel has been set to: %d\n", noteChannel);
            }
            break;
        case SET_COLOR_PRESSED:
            button = buffer[1];
            if (button >= 0 && button < DEV_MAX_BTN) {
                btnConfig[button].colorPressed.red = buffer[2];
                btnConfig[button].colorPressed.green = buffer[3];
                btnConfig[button].colorPressed.blue = buffer[4];
                storeButtonConfig(button, btnConfig[buffer[1]]);
                DEBUG_PRINTF("Button %d pressed LED color has been set to: 0x%02x%02x%02x\n", button + 1, buffer[2], buffer[3], buffer[4]);
            }
            break;
        case SET_COLOR_RELEASED:
            button = buffer[1];
            if (button >= 0 && button < DEV_MAX_BTN) {
                btnConfig[button].colorReleased.red = buffer[2];
                btnConfig[button].colorReleased.green = buffer[3];
                btnConfig[button].colorReleased.blue = buffer[4];
                storeButtonConfig(button, btnConfig[button]);
                DEBUG_PRINTF("Button %d released LED color has been set to: 0x%02x%02x%02x\n", button + 1, buffer[2], buffer[3], buffer[4]);
            }
            break;
        case SET_MIDI_NOTE:
            button = buffer[1];
            if (button >= 0 && button < DEV_MAX_BTN) {
                if (buffer[2] >= 0 && buffer[2] <= MIDI_MAX_NOTE) {
                    btnConfig[button].midiNote = buffer[2];
                    storeButtonConfig(button, btnConfig[button]);
                    DEBUG_PRINTF("Button %d MIDI note has been set to: %d\n", button + 1, buffer[2]);
                }
            }
            break;
        case SET_MIDI_VELOCITY:
            button = buffer[1];
            if (button >= 0 && button < DEV_MAX_BTN) {
                if (buffer[2] >= 0 && buffer[2] <= MIDI_MAX_VELOCITY) {
                    btnConfig[button].midiVelocity = buffer[2];
                    storeButtonConfig(button, btnConfig[button]);
                    DEBUG_PRINTF("Button %d MIDI velocity has been set to: %d\n", button + 1, buffer[2]);
                }
            }
            break;
        case SET_MOMENTARY:
            button = buffer[1];
            if (button >= 0 && button < DEV_MAX_BTN) {
                if (buffer[2] == 0 || buffer[2] == 1) {
                    btnConfig[button].momentary = buffer[2];
                    storeButtonConfig(button, btnConfig[button]);
                    DEBUG_PRINTF("Button %d CC flag has been set to: %d\n", button + 1, buffer[2]);
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

void Device::storeButtonConfig(uint8_t button, Device::btnConfig_t cfg) {
    uint16_t offset = ADDR_CONFIG + button * sizeof(Device::btnConfig_t);
    EEPROM.put(offset, cfg);
}

Device::btnConfig_t Device::fetchButtonConfig(uint8_t button) {
    uint16_t offset = ADDR_CONFIG + button * sizeof(Device::btnConfig_t);
    Device::btnConfig_t cfg;
    EEPROM.get(offset, cfg);
    return cfg;
}

void Device::storeByte(uint16_t addr, uint8_t data) {
    EEPROM.put(addr, data);
}

uint8_t Device::fetchByte(uint16_t addr) {
    uint8_t data;
    EEPROM.get(addr, data);
    return data;
}