/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_init.h"
#include "ss_leds.h"
#include "ss_clock.h"
#include <libopencm3/cm3/scb.h>

inline SS_FEEDBACK ss_init() {
    SCB_VTOR = 0x08020200;

    ss_clock_init(SS_CLOCK_FAST);

    ss_leds_init();



    return SS_FEEDBACK_OK;
}