#include "ss_init.h"
#include "ss_leds.h"
#include <libopencm3/cm3/scb.h>

SS_FEEDBACK ss_init() {
    SCB_VTOR = 0x08020200;

    ss_leds_init();

    return SS_FEEDBACK_OK;
}