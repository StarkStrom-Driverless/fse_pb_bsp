#include "ss_gpio.h"

static uint8_t sim_pin_state[256];
static uint8_t sim_pin_mode[256];

bool ss_io_init(uint16_t pin_id, uint8_t mode) {
    sim_pin_mode[(uint8_t) pin_id] = mode;

    return true;
}

void ss_io_write(uint16_t pin_id, uint8_t value) {
    uint8_t idx = (uint8_t) pin_id;

    if (value == SS_GPIO_TOGGLE) {
        sim_pin_state[idx] = !sim_pin_state[idx];
    } else {
        sim_pin_state[idx] = (value == SS_GPIO_ON) ? 1 : 0;
    }
}

uint16_t ss_io_read(uint16_t pin_id) {
    return sim_pin_state[(uint8_t) pin_id];
}
