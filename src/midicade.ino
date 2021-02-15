// Copyright © 2021 Cosku Bas. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE.md file.

#include "debug.h"
#include "device.h"

const deviceConfig_t cfg = {
    .mosiPin = 0,
    .sckPin  = 1,
    .csPin   = 2,
    .btnPins = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 17, 18, 19 },
};

Device* midicade;

void setup() {
    DEBUG_INIT();
    DEBUG_PRINTF("Midicade has been started in debug mode\n");
    midicade = new Device(&cfg);
}

void loop() {
    midicade->UpdateState();
}