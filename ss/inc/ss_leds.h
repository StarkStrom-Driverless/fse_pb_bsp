/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_LEDS

#ifndef _SS_LEDS_H_
#define _SS_LEDS_H_

#include <stdbool.h>

#ifdef USE_PRIVATE
bool ss_leds_init(void);
#endif

void ss_led_error_on(void);
void ss_led_error_off(void);
void ss_led_error_toggle(void);

void ss_led_heartbeat_on(void);
void ss_led_heartbeat_off(void);
void ss_led_heartbeat_toggle(void);

void ss_led_dbg1_on(void);
void ss_led_dbg1_off(void);
void ss_led_dbg1_toggle(void);

void ss_led_dbg2_on(void);
void ss_led_dbg2_off(void);
void ss_led_dbg2_toggle(void);

#endif // _SS_LEDS_H_

#endif // COMPILE_SS_LEDS
