#ifndef _SS_WATCHDOG_H_
#define _SS_WATCHDOG_H_

#include "ss_feedback.h"
#include <inttypes.h>

SS_FEEDBACK ss_watchdog_init(uint16_t period_ms);
SS_FEEDBACK ss_watchdog_feed() ;

#endif