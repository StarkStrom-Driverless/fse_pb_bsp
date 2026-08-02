/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#define USE_PRIVATE
#include "ss_config.h"

#if COMPILE_SS_INIT
#include "ss_init.h"
#include "ss_leds.h"
#include "ss_clock.h"
#include "ss_error.h"
#include <libopencm3/cm3/scb.h>

inline bool ss_init() {
    SCB_VTOR = 0x08020200;

    if (!ss_clock_init(SS_CLOCK_FAST)) SS_ERROR(NULL);

    if (!ss_leds_init()) SS_ERROR(NULL);

    return true;
}
#endif // COMPILE_SS_INIT
