#ifndef _SS_FREQ_MEASURE_
#define _SS_FREQ_MEASURE_

#include <inttypes.h>

struct FREQ_CHANNEL {
    volatile uint32_t frequency;
    volatile uint32_t last_capture;
    uint32_t resolution;
};


struct SS_FREQ_MEASURE {
    struct FREQ_CHANNEL freq_block[64];
};

extern struct SS_FREQ_MEASURE ss_freq_measure;



#endif