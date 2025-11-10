/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#ifndef _SS_DELAY_H_
#define _SS_DELAY_H_

#include <inttypes.h>
#include "ss_clock.h"

static inline void ss_delay(volatile uint32_t count) {
    count *= ss_clock.ahb * 10;
    while (count--) {
        __asm__ __volatile__("nop");
    }
}

#endif