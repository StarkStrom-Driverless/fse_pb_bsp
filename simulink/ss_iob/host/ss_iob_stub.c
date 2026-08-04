#include <inttypes.h>
#include "ss_iob.h"

struct IOB ss_iob;

static uint8_t sim_iob_value[256];

bool ss_iob_add(uint16_t pin_id, uint8_t polarity) {
    sim_iob_value[(uint8_t) pin_id] = 0;

    return true;
}

uint8_t ss_iob_get(uint16_t pin_id) {
    return sim_iob_value[(uint8_t) pin_id];
}
