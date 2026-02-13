#include "ss_watchdog.h"
#include <libopencm3/stm32/iwdg.h>

#define SS_FEEDBACK_BASE                            SS_FEEDBACK_BASE_NOT_SET


SS_FEEDBACK ss_watchdog_init(uint16_t period_ms) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    iwdg_set_period_ms(period_ms);
    iwdg_start();

    return rc;
}

SS_FEEDBACK ss_watchdog_feed() {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    iwdg_reset();

    return rc;
}