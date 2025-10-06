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