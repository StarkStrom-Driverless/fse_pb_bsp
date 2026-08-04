#include "ss_fm.h"

struct SS_FREQ_MEASURE ss_fm;

static uint8_t sim_fm_enabled[256];

bool ss_fm_init(uint16_t pin_id, uint32_t resolution) {
    sim_fm_enabled[(uint8_t) pin_id] = 1;

    return true;
}

bool ss_fm_read(uint16_t pin_id, float *value) {
    if (value == 0) {
        return false;
    }

    *value = sim_fm_enabled[(uint8_t) pin_id] ? 100.0f : 0.0f;

    return true;
}
