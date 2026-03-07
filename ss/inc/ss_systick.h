/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_SYSTICK

#ifndef _SS_SYSTICK_H_
#define _SS_SYSTICK_H_

#include <inttypes.h>

#ifndef SS_USE_RTOS
#ifdef USE_PRIVATE
void sys_tick_handler(void);
#endif
#endif

#endif // _SS_SYSTICK_H_

#endif // COMPILE_SS_SYSTICK
