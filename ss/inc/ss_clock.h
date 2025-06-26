#ifndef _SS_CLOCK_H_
#define _SS_CLOCK_H_

#include <inttypes.h>

#include <libopencm3/stm32/can.h>

#define SS_CLOCK_DEFAULT 0
#define SS_CLOCK_FAST 1

struct SS_CLOCK {
    uint16_t ahb;
    uint16_t apb1;
    uint16_t apb2;
};

struct SS_CLOCK_CAN {
    uint32_t prescaler;
    uint32_t tseg1;
    uint32_t tseg2;
    uint32_t sjw;
};

extern struct SS_CLOCK ss_clock;

struct SS_CLOCK* ss_clock_init(uint8_t config);

uint8_t ss_clock_can(struct SS_CLOCK_CAN* config, uint32_t baudrate, struct SS_CLOCK* clock_config);

#endif