// Copyright © 2021 Cosku Bas. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE.md file.

#ifndef __DEBUG_H__
#define __DEBUG_H__

//#define MIDICADE_DEBUG 1

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
