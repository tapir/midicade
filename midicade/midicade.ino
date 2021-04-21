// Copyright © 2021 Cosku Bas. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE.md file.

#include "debug.h"
#include "device.h"

Device* midicade;

void setup() {
    DEBUG_INIT();
    DEBUG_PRINTF("Midicade has been started in debug mode\n");
    midicade = new Device();
}

void loop() {
    midicade->UpdateState();
}