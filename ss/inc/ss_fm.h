/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_FM

#ifndef _SS_FREQ_MEASURE_
#define _SS_FREQ_MEASURE_

#include <inttypes.h>

#include "ss_clock.h"

struct FREQ_PIN {
    volatile uint32_t last_capture;
    volatile float frequency;
    uint8_t init;
    uint32_t resolution;
    uint16_t pin_id;
    uint8_t enabled;
    uint8_t requested;
    uint32_t timer;
    uint32_t ic;
};

struct FREQ_PORT {
    struct FREQ_PIN pins[4];
};


struct SS_FREQ_MEASURE {
    struct FREQ_PORT ports[6];
};

extern struct SS_FREQ_MEASURE ss_fm;

/***
 *
 * PERIPH FUNCTIONS
 *
 */
#ifdef USE_PRIVATE
SS_FEEDBACK ss_fm_get_ic_from_pin_id(uint16_t pin_id, uint32_t* ic_channel);
#endif
#ifdef USE_PRIVATE
SS_FEEDBACK ss_fm_get_iqr_cc_from_pin_id(uint16_t pin_id, uint32_t* irq);
#endif
#ifdef USE_PRIVATE
SS_FEEDBACK ss_fm_get_irq_from_pin_id(uint16_t pin_id, uint32_t* irq);
#endif
#ifdef USE_PRIVATE
SS_FEEDBACK ss_fm_get_pin_struct_from_pin_id(uint16_t pin_id, struct FREQ_PIN** tmp);
#endif
#ifdef USE_PRIVATE
uint8_t ss_fm_get_af_from_pin_id(uint16_t pin_id);
#endif
#ifdef USE_PRIVATE
SS_FEEDBACK ss_fm_get_ti_from_pin_id(uint16_t pin_id, uint32_t* ti);
#endif

/***
 *
 * USER FUNCTIONS
 *
 */

SS_FEEDBACK ss_fm_init(uint16_t pin_id,  uint32_t resolution);

SS_FEEDBACK ss_fm_read(uint16_t pin_id, float *value);

/***
 *
 * ISR FUNCTIONS
 *
 */
#ifdef USE_PRIVATE
void ss_fm_isr(uint32_t timer, uint8_t port_id);
#endif
#ifdef USE_PRIVATE
void tim2_isr(void);
#endif
#ifdef USE_PRIVATE
void tim1_cc_isr(void);
#endif

#endif // _SS_FREQ_MEASURE_

#endif // COMPILE_SS_FM
