#ifndef _SS_ADC_H_
#define _SS_ADC_H_

#include <inttypes.h>

struct SS_ADC {
    uint16_t adc_request_channel;
    // 0 ADC-Converstion finished
    // 1 ADC-Converstion requested
    // 2 ADC-Converstion was read
    uint8_t adc_converstion_status;
    uint16_t adc_value;
};

int8_t ss_enable_adc_clock_from_pin_id(uint16_t pin_id);

uint32_t ss_get_adc_from_pin_id(uint16_t pin_id);


uint8_t ss_get_adc_channel_from_pin_id(uint16_t pin_id);

uint16_t ss_adc_init(uint16_t pin_id, struct SS_ADC* adc_struct);

int8_t ss_adc_start(uint16_t pin_id, struct SS_ADC* adc_struct);

int8_t ss_adc_done(struct SS_ADC* adc_struct);

uint16_t ss_adc_read(struct SS_ADC* adc_struct);

void adc1_2_isr(void);

void adc3_isr(void);

#endif