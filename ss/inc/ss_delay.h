#ifndef _SS_DELAY_H_
#define _SS_DELAY_H_

#include <inttypes.h>

static inline void delay(volatile uint32_t count) {
    while (count--) {
        __asm__ __volatile__("nop");
    }
}

#endif