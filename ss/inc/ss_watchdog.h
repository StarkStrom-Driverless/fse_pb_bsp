#include "ss_config.h"

#if COMPILE_SS_WATCHDOG

#ifndef _SS_WATCHDOG_H_
#define _SS_WATCHDOG_H_

#include "ss_feedback.h"
#include <inttypes.h>
#include <stdbool.h>

bool ss_watchdog_init(uint16_t period_ms);
bool ss_watchdog_feed();

#endif // _SS_WATCHDOG_H_

#endif // COMPILE_SS_WATCHDOG
