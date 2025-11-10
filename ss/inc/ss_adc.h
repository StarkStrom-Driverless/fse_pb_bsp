/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#ifndef _SS_ADC_H_
#define _SS_ADC_H_

#include <inttypes.h>
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
void adc_isr(void);


/***
 * 
 * ADC USER FUNCTIONS
 * 
 */
SS_FEEDBACK ss_adc_read(uint16_t pin_id, uint16_t *val);
SS_FEEDBACK ss_adc_init(uint16_t pin_id);


/***
 * 
 * ADC PERIPH FUNCTIONS
 * 
 */
SS_FEEDBACK ss_adc_rcc_init_from_pin_id(uint16_t pin_id);
uint32_t ss_adc_get_adc_from_pin_id(uint16_t pin_id);
uint32_t ss_adc_get_channel_from_pin_id(uint16_t pin_id);
SS_FEEDBACK ss_adc_get_measurement_pos_from_pin_id(uint16_t pin_id, uint8_t *measurement_pos);


/***
 * 
 * ADC CORE CIRCULAR MEASUREMENT
 * 
 */
SS_FEEDBACK ss_adc_set_next_measurment_pos(void);
SS_FEEDBACK ss_adc_start(void);
SS_FEEDBACK ss_adc_start_channel(uint16_t pin_id);



#endif