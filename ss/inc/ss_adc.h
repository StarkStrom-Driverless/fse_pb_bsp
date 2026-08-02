/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_ADC

#ifndef _SS_ADC_H_
#define _SS_ADC_H_

#include <inttypes.h>
#include <stdbool.h>
#include "ss_feedback.h"

#define MAX_MEASUREMENT 12

struct ADC_MEASSUREMENT {
    uint16_t measurement;
    uint8_t enable;
    uint16_t pin_id;
};

struct SS_ADC {
    struct ADC_MEASSUREMENT measurements[MAX_MEASUREMENT];
    uint8_t measurement_pos;

};

extern struct SS_ADC ss_adc;

/***
 *
 * ISR FUNCTIONS
 *
 */
#ifdef USE_PRIVATE
void adc_isr(void);
#endif


/***
 *
 * ADC USER FUNCTIONS
 *
 */

bool ss_adc_read(uint16_t pin_id, uint16_t *val);

bool ss_adc_init(uint16_t pin_id);



/***
 *
 * ADC PERIPH FUNCTIONS
 *
 */
#ifdef USE_PRIVATE
bool ss_adc_rcc_init_from_pin_id(uint16_t pin_id);
#endif
#ifdef USE_PRIVATE
bool ss_adc_get_adc_from_pin_id(uint16_t pin_id, uint32_t *adc);
#endif
#ifdef USE_PRIVATE
bool ss_adc_get_channel_from_pin_id(uint16_t pin_id, uint32_t *adc_channel);
#endif
#ifdef USE_PRIVATE
bool ss_adc_get_measurement_pos_from_pin_id(uint16_t pin_id, uint8_t *measurement_pos);
#endif


/***
 *
 * ADC CORE CIRCULAR MEASUREMENT
 *
 */
#ifdef USE_PRIVATE
bool ss_adc_set_next_measurment_pos(void);
#endif
#ifdef USE_PRIVATE
bool ss_adc_start(void);
#endif
#ifdef USE_PRIVATE
bool ss_adc_start_channel(uint16_t pin_id);
#endif

#endif // _SS_ADC_H_

#endif // COMPILE_SS_ADC
