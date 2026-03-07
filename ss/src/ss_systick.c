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

#if COMPILE_SS_SYSTICK
#include <libopencm3/cm3/systick.h>
#include "ss_systick.h"
#include <inttypes.h>



#ifndef SS_USE_RTOS
void sys_tick_handler(void) {

}
#endif


#endif // COMPILE_SS_SYSTICK
