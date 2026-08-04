#include "ss_pwm.h"

static uint32_t sim_pwm_freq[256];
static uint32_t sim_pwm_duty[256];

bool ss_pwm_init(uint16_t pin_id, uint32_t frequency) {
    sim_pwm_freq[(uint8_t) pin_id] = frequency;
    sim_pwm_duty[(uint8_t) pin_id] = 0;

    return true;
}

bool ss_pwm_write(uint16_t pin_id, uint32_t value) {
    if (value > 100) {
        value = 100;
    }

    sim_pwm_duty[(uint8_t) pin_id] = value;

    return true;
}
