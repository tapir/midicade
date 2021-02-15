// Copyright © 2021 Cosku Bas. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE.md file.

#ifndef __DEBUG_H__
#define __DEBUG_H__

#define BUTTON_01 0
#define BUTTON_02 1
#define BUTTON_03 2
#define BUTTON_04 3
#define BUTTON_05 4
#define BUTTON_06 5
#define BUTTON_07 6
#define BUTTON_08 7
#define BUTTON_09 8
#define BUTTON_10 9
#define BUTTON_11 10
#define BUTTON_12 11
#define BUTTON_13 12
#define BUTTON_14 13
#define BUTTON_15 14
#define BUTTON_16 15

#ifdef MIDICADE_DEBUG

#define PRINTF_DISABLE_SUPPORT_LONG_LONG
#define PRINTF_DISABLE_SUPPORT_PTRDIFF_T
#define PRINTF_DISABLE_SUPPORT_FLOAT
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL

#include <LibPrintf.h>

#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#define DEBUG_INIT()      \
    Serial.begin(115200); \
    while (!Serial) { }
#else

#define DEBUG_PRINTF(...)
#define DEBUG_INIT()

#endif

#endif