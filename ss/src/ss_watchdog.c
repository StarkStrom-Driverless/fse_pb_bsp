#define USE_PRIVATE
#include "ss_config.h"

#if COMPILE_SS_WATCHDOG
#include "ss_watchdog.h"
#include <libopencm3/stm32/iwdg.h>


bool ss_watchdog_init(uint16_t period_ms) {
    iwdg_set_period_ms(period_ms);
    iwdg_start();

    return true;
}

bool ss_watchdog_feed() {
    iwdg_reset();

    return true;
}
#endif // COMPILE_SS_WATCHDOG
