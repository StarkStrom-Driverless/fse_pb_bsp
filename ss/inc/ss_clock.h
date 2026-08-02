/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_CLOCK

#ifndef _SS_CLOCK_H_
#define _SS_CLOCK_H_

#include <inttypes.h>
#include <stdbool.h>

#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/spi.h>

#include "ss_feedback.h"

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

#ifdef USE_PRIVATE
SS_FEEDBACK ss_clock_init(uint8_t config);
#endif

#ifdef USE_PRIVATE
bool ss_clock_can(struct SS_CLOCK_CAN* config, uint32_t baudrate);
#endif

#ifdef USE_PRIVATE
SS_FEEDBACK ss_clock_spi(uint32_t* prescaler, uint32_t baudrate, uint8_t interface);
#endif

#ifdef USE_PRIVATE
SS_FEEDBACK ss_clock_fm(uint16_t pin_id, uint32_t *frequency);
#endif

#endif // _SS_CLOCK_H_

#endif // COMPILE_SS_CLOCK
