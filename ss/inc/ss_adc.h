#ifndef _SS_ADC_H_
#define _SS_ADC_H_

#include <inttypes.h>

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

uint16_t ss_adc_get_measurement_pos_from_pin_id(uint16_t pin_id);

int8_t ss_enable_adc_clock_from_pin_id(uint16_t pin_id);

uint32_t ss_get_adc_from_pin_id(uint16_t pin_id);

uint8_t ss_adc_set_next_measurment_pos(void);

uint8_t ss_get_adc_channel_from_pin_id(uint16_t pin_id);

uint16_t ss_adc_init(uint16_t pin_id);

int8_t ss_adc_start(void);

int8_t ss_adc_start_channel(uint16_t pin_id);

uint16_t ss_adc_read(uint16_t pin_id);

void adc1_2_isr(void);

void adc3_isr(void);

#endif