#include "ss_init.h"
#include "ss_leds.h"
#include "ss_clock.h"
#include <libopencm3/cm3/scb.h>

inline SS_FEEDBACK ss_init() {
    

    ss_clock_init(SS_CLOCK_FAST);

    ss_leds_init();



    return SS_FEEDBACK_OK;
}