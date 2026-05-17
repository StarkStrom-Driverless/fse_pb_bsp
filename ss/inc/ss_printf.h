/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_PRINTF

#ifndef _SS_PRINTF_H_
#define _SS_PRINTF_H_

#include <inttypes.h>

void ss_printf(uint8_t uart_interface, const char* fmt, ...);

#endif // _SS_PRINTF_H_

#endif // COMPILE_SS_PRINTF
