#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/cm3/nvic.h>
#include <inttypes.h>
#include "ss_makros.h"
#include "ss_gpio.h"
#include "ss_adc.h"

extern struct SS_ADC ss_adc;

int8_t ss_enable_adc_clock_from_pin_id(uint16_t pin_id) {
    int8_t status = 0;
    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 4):
        case PIN('A', 5):
            rcc_periph_clock_enable(RCC_ADC1);
            break;

        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1):
            rcc_periph_clock_enable(RCC_ADC2);
            break;

        case PIN('A', 2):
        case PIN('A', 3):
        case PIN('C', 2):
        case PIN('C', 3):
            rcc_periph_clock_enable(RCC_ADC3);
            break;

        default: status = -1; break;
    }

    return status;
}

uint32_t ss_get_adc_from_pin_id(uint16_t pin_id) {
    uint32_t adc = 0;

    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 4):
        case PIN('A', 5):
            adc = ADC1;
            break;

        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1):
            adc = ADC2;
            break;

        case PIN('A', 2):
        case PIN('A', 3):
        case PIN('C', 2):
        case PIN('C', 3):
            adc = ADC3;
            break;

        default: break;
    }

    return adc;
}


uint8_t ss_get_adc_channel_from_pin_id(uint16_t pin_id) {
    uint8_t adc_channel = 0;

    switch(pin_id) {
        case PIN('A', 0): adc_channel = ADC_CHANNEL0; break;
        case PIN('A', 1): adc_channel = ADC_CHANNEL1; break;
        case PIN('A', 4): adc_channel = ADC_CHANNEL4; break;
        case PIN('A', 5): adc_channel = ADC_CHANNEL5; break;

        case PIN('A', 6): adc_channel = ADC_CHANNEL6; break;
        case PIN('A', 7): adc_channel = ADC_CHANNEL7; break;
        case PIN('B', 0): adc_channel = ADC_CHANNEL8; break;
        case PIN('B', 1): adc_channel = ADC_CHANNEL9; break;

        case PIN('A', 2): adc_channel = ADC_CHANNEL2; break;
        case PIN('A', 3): adc_channel = ADC_CHANNEL3; break;
        case PIN('C', 2): adc_channel = ADC_CHANNEL12; break;
        case PIN('C', 3): adc_channel = ADC_CHANNEL13; break;

        default: break;
    }

    return adc_channel;
}

int8_t ss_adc_init(uint16_t pin_id, struct SS_ADC* adc_struct) {
    adc_struct->adc_converstion_status = 2;

    ss_enable_adc_clock_from_pin_id(pin_id);

    ss_io_init(pin_id, GPIO_MODE_ANALOG);

    uint32_t adc = ss_get_adc_from_pin_id(pin_id);

    adc_power_off(adc);

    //adc_reset_calibration(adc);

    //adc_calibration(adc);

    adc_power_on(adc);

    adc_set_sample_time_on_all_channels(adc, ADC_SMPR_SMP_28CYC);

    adc_enable_eoc_interrupt(adc);

    nvic_enable_irq(NVIC_ADC_IRQ);

    return 0;
}

int8_t ss_adc_start(uint16_t pin_id, struct SS_ADC* adc_struct) {
    if (adc_struct->adc_converstion_status == 0 || adc_struct->adc_converstion_status == 1) return -1;

    uint32_t adc = ss_get_adc_from_pin_id(pin_id);

    uint8_t adc_channel = ss_get_adc_channel_from_pin_id(pin_id);

    adc_set_regular_sequence(adc, 1, &adc_channel);

    adc_struct->adc_request_channel = pin_id;

    adc_struct->adc_converstion_status = 1;

    adc_start_conversion_regular(adc);
}

int8_t ss_adc_done(struct SS_ADC* adc_struct) {
    return (adc_struct->adc_converstion_status == 0)?1:0;
}

uint16_t ss_adc_read(struct SS_ADC* adc_struct) {
    adc_struct->adc_converstion_status = 2;
    return adc_struct->adc_value;
}

void adc_isr(void) {
    if (adc_eoc(ADC1)) {
        ss_adc.adc_value = adc_read_regular(ADC1);
        ss_adc.adc_converstion_status = 0;
    }
    else if (adc_eoc(ADC2)) {
        ss_adc.adc_value = adc_read_regular(ADC2);
        ss_adc.adc_converstion_status = 0;
    }
    else if (adc_eoc(ADC3)) {
        ss_adc.adc_value = adc_read_regular(ADC3);
        ss_adc.adc_converstion_status = 0;
    }
}


