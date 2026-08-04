#include "ss_pwm.h"

static uint32_t sim_pwm_freq[256];
static uint32_t sim_pwm_duty[256];
static uint32_t sim_pwm_period[256];

bool ss_pwm_init(uint16_t pin_id, uint32_t frequency) {
    sim_pwm_freq[(uint8_t) pin_id] = frequency;
    sim_pwm_duty[(uint8_t) pin_id] = 0;
    sim_pwm_period[(uint8_t) pin_id] = 100;

    return true;
}

bool ss_pwm_init_highres(uint16_t pin_id, uint32_t frequency) {
    sim_pwm_freq[(uint8_t) pin_id] = frequency;
    sim_pwm_duty[(uint8_t) pin_id] = 0;
    sim_pwm_period[(uint8_t) pin_id] = 1000;

    return true;
}

static bool sim_pwm_write(uint16_t pin_id, uint32_t value) {
    uint8_t idx = (uint8_t) pin_id;

    if (value > sim_pwm_period[idx]) {
        value = sim_pwm_period[idx];
    }

    sim_pwm_duty[idx] = value;

    return true;
}

bool ss_pwm_write(uint16_t pin_id, uint32_t value) {
    return sim_pwm_write(pin_id, value);
}

bool ss_pwm_write_highres(uint16_t pin_id, uint32_t value) {
    return sim_pwm_write(pin_id, value);
}
