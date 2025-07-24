#ifndef _SS_FREQ_MEASURE_
#define _SS_FREQ_MEASURE_

#include <inttypes.h>

#include "ss_clock.h"

struct FREQ_PIN {
    volatile uint32_t last_capture;
    uint32_t resolution;
    uint16_t pin_id;
    uint8_t enabled;
    uint8_t requested;

};

struct FREQ_PORT {
    struct FREQ_PIN pins[4];
};


struct SS_FREQ_MEASURE {
    struct FREQ_PORT ports[6];
};

extern struct SS_FREQ_MEASURE ss_freq_measure;


struct FREQ_PIN* ss_freq_measure_init(uint16_t pin_id, struct SS_CLOCK* clock, uint32_t resolution);

uint32_t ss_freq_measure_get(uint16_t pin_id);


#endif