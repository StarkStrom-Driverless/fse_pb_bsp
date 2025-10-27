#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/cm3/nvic.h>
#include <inttypes.h>
#include "ss_makros.h"
#include "ss_gpio.h"
#include "ss_adc.h"

struct SS_ADC ss_adc;

#define SS_FEEDBACK_BASE SS_FEEDBACK_ADC_INIT_ERROR

/***
 * 
 * ISR FUNCTIONS
 * 
 */
void adc_isr(void) {
    if (adc_eoc(ADC1)) {
        ss_adc.measurements[ss_adc.measurement_pos].measurement = adc_read_regular(ADC1);
    }
    else if (adc_eoc(ADC2)) {
        ss_adc.measurements[ss_adc.measurement_pos].measurement = adc_read_regular(ADC2);
    }
    else if (adc_eoc(ADC3)) {
        ss_adc.measurements[ss_adc.measurement_pos].measurement = adc_read_regular(ADC3);
    }
    if (ss_adc_set_next_measurment_pos()) {
        ss_adc_start_channel(ss_adc.measurements[ss_adc.measurement_pos].pin_id);
    }
}


/***
 * 
 * ADC USER FUNCTIONS
 * 
 */
SS_FEEDBACK ss_adc_read(uint16_t pin_id, uint16_t *val) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    uint8_t measure_pos = 0;

    rc = ss_adc_get_measurement_pos_from_pin_id(pin_id, &measure_pos);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    *val = ss_adc.measurements[measure_pos].measurement;

    return rc;
}

SS_FEEDBACK ss_adc_init(uint16_t pin_id) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    uint8_t measurement_pos = 0;

    rc = ss_adc_rcc_init_from_pin_id(pin_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_io_init(pin_id, GPIO_MODE_ANALOG);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint32_t adc = ss_adc_get_adc_from_pin_id(pin_id);
    SS_HANDLE_NULL_WITH_EXIT(adc);

    adc_power_off(adc);

    adc_power_on(adc);

    adc_set_sample_time_on_all_channels(adc, ADC_SMPR_SMP_28CYC);

    adc_enable_eoc_interrupt(adc);

    nvic_enable_irq(NVIC_ADC_IRQ);

    rc = ss_adc_get_measurement_pos_from_pin_id(pin_id, &measurement_pos);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    ss_adc.measurements[measurement_pos].enable = 1;
    ss_adc.measurements[measurement_pos].pin_id = pin_id;
    ss_adc.measurement_pos = 0;

    rc = ss_adc_start();

    return rc;
}


/***
 * 
 * ADC PERIPH FUNCTIONS
 * 
 */
SS_FEEDBACK ss_adc_rcc_init_from_pin_id(uint16_t pin_id) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

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

        default: 
            rc = SS_FEEDBACK_ADC_RCC_INIT_ERROR; 
            break;
    }

    return rc;
}

uint32_t ss_adc_get_adc_from_pin_id(uint16_t pin_id) {
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


uint32_t ss_adc_get_channel_from_pin_id(uint16_t pin_id) {
    uint32_t adc_channel = 0;

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

SS_FEEDBACK ss_adc_get_measurement_pos_from_pin_id(uint16_t pin_id, uint8_t *measurement_pos) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    
    switch(pin_id) {
        case PIN('A', 0): *measurement_pos = 0; break;
        case PIN('A', 1): *measurement_pos = 1; break;
        case PIN('A', 4): *measurement_pos = 2; break;
        case PIN('A', 5): *measurement_pos = 3; break;

        case PIN('A', 6): *measurement_pos = 4; break;
        case PIN('A', 7): *measurement_pos = 5; break;
        case PIN('B', 0): *measurement_pos = 6; break;
        case PIN('B', 1): *measurement_pos = 7; break;

        case PIN('A', 2): *measurement_pos = 8; break;
        case PIN('A', 3): *measurement_pos = 9; break;
        case PIN('C', 2): *measurement_pos = 10; break;
        case PIN('C', 3): *measurement_pos = 11; break;

        default: 
            rc = SS_FEEDBACK_ADC_PINID_ERROR;    
            break;
    }

    return rc;
}




/***
 * 
 * ADC CORE CIRCULAR MEASUREMENT
 * 
 */
SS_FEEDBACK ss_adc_set_next_measurment_pos(void) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    uint8_t original_pos = ss_adc.measurement_pos;
    do {
        ss_adc.measurement_pos++;
        if (ss_adc.measurement_pos >= MAX_MEASUREMENT) {
            ss_adc.measurement_pos = 0;
        }
        if (ss_adc.measurement_pos == original_pos) {
            rc = SS_FEEDBACK_ADC_FAILED_NEXT_MPOS;
            break;
        }
    }while(ss_adc.measurements[ss_adc.measurement_pos].enable == 0);

    return rc;
}

SS_FEEDBACK ss_adc_start(void) {
    SS_FEEDBACK rc = SS_FEEDBACK_ADC_FAILED_START;

    for (uint8_t i = 0; i < MAX_MEASUREMENT; i++) {
        if (ss_adc.measurements[i].enable == 1) {
            rc = SS_FEEDBACK_OK;
        }
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_adc_set_next_measurment_pos();
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint16_t pin_id = ss_adc.measurements[ss_adc.measurement_pos].pin_id;
    rc = ss_adc_start_channel(pin_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    return rc;
}

SS_FEEDBACK ss_adc_start_channel(uint16_t pin_id) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    uint32_t adc = ss_adc_get_adc_from_pin_id(pin_id);
    SS_HANDLE_NULL_WITH_EXIT(adc);

    uint32_t adc_channel = ss_adc_get_channel_from_pin_id(pin_id);
    SS_HANDLE_NULL_WITH_EXIT(adc_channel);

    adc_set_regular_sequence(adc, 1, &adc_channel);

    adc_start_conversion_regular(adc);

    return rc;
}








