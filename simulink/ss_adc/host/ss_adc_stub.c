#include "ss_adc.h"

struct SS_ADC ss_adc;

static uint8_t sim_adc_enabled[256];

bool ss_adc_init(uint16_t pin_id) {
    sim_adc_enabled[(uint8_t) pin_id] = 1;

    return true;
}

bool ss_adc_read(uint16_t pin_id, uint16_t *val) {
    if (val == 0) {
        return false;
    }

    *val = sim_adc_enabled[(uint8_t) pin_id] ? 2048 : 0;

    return true;
}
