/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include "ss_gpio.h"
#include "ss_pwm.h"
#include "ss_clock.h"
#include "ss_feedback.h"

#define SS_FEEDBACK_BASE SS_FEEDBACK_IO_INIT_ERROR

SS_FEEDBACK ss_get_timer_channel_from_pin_id(uint16_t pin_id, uint32_t* timer_addr) {
    SS_FEEDBACK rc = 0;

    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 5):
        case PIN('A', 6):
        case PIN('A', 8):
        case PIN('A', 15):
        case PIN('B', 14):
        case PIN('C', 6):  
            *timer_addr = TIM_OC1; 
            break;

        case PIN('A', 2):
        case PIN('A', 10):
        case PIN('B', 0):
        case PIN('B', 10):
        case PIN('C', 8):  
            *timer_addr = TIM_OC3; 
            break;

        case PIN('A', 3):
        case PIN('A', 11):
        case PIN('B', 1):
        case PIN('B', 11):
        case PIN('C', 9):
            *timer_addr = TIM_OC4; 
            break;

        case PIN('A', 1):
        case PIN('A', 7):
        case PIN('A', 9):
        case PIN('B', 15):
        case PIN('C', 7):  
            *timer_addr = TIM_OC2; 
            break;

        default:
            rc = SS_FEEDBACK_PWM_PIN_ID_ERROR; 
            break;
    }

    return rc;
}

SS_FEEDBACK ss_get_timer_from_pin_id(uint16_t pin_id, uint32_t* timer_addr) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    switch(pin_id) {
        case PIN('A', 0):           // CH1
        case PIN('A', 1):           // CH2
        case PIN('A', 2):           // CH3
        case PIN('A', 3):           // CH4
            *timer_addr = TIM5; 
            break; 

        case PIN('A', 5):           // CH1
        case PIN('A', 15):
        case PIN('B', 10):
        case PIN('B', 11): 
            *timer_addr = TIM2; 
            break;

        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1):  
            *timer_addr = TIM3; 
            break;

        case PIN('A', 8):
        case PIN('A', 9):
        case PIN('A', 10):
        case PIN('A', 11): 
            *timer_addr = TIM1; 
            break;

        case PIN('B', 14):
        case PIN('B', 15): 
            *timer_addr = TIM12; 
            break;

        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9):  
            *timer_addr = TIM8; 
            break;

        default:
            rc = SS_FEEDBACK_PWM_PIN_ID_ERROR; 
            break;
    }

    return rc;
}

SS_FEEDBACK ss_enable_timer_clock_from_pin_id(uint16_t pin_id) {
    SS_FEEDBACK status = SS_FEEDBACK_OK;
    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 2):
        case PIN('A', 3):
            rcc_periph_clock_enable(RCC_TIM5);
            break;

        case PIN('A', 5):
        case PIN('A', 15):
        case PIN('B', 10):
        case PIN('B', 11):
            rcc_periph_clock_enable(RCC_TIM2);
            break;

        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1):
            rcc_periph_clock_enable(RCC_TIM3);
            break;

        case PIN('A', 8):
        case PIN('A', 9):
        case PIN('A', 10):
        case PIN('A', 11):
            rcc_periph_clock_enable(RCC_TIM1);
            break;

        case PIN('B', 14):
        case PIN('B', 15):
            rcc_periph_clock_enable(RCC_TIM12);
            break;

        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9):
            rcc_periph_clock_enable(RCC_TIM8);
            break;

        default:
            status = SS_FEEDBACK_IO_PINID_ERROR;
            break;
    }
    return status;
}

SS_FEEDBACK get_pwm_af_mode_for_pin_id(uint16_t pin_id, uint8_t* af) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 2):
        case PIN('A', 3):
        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1):
            *af = 2;
            break;

        case PIN('A', 5):
        case PIN('A', 8):
        case PIN('A', 9):
        case PIN('A', 10):
        case PIN('A', 11):
        case PIN('A', 15):
        case PIN('B', 10):
        case PIN('B', 11):
            *af = 1;
            break;

        case PIN('B', 14):
        case PIN('B', 15):
            *af = 9;
            break;

        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9):
            *af = 3;
            break;

        default:
            *af = SS_FEEDBACK_PWM_PIN_ID_ERROR;
            break;
    }
    return rc;
}

SS_FEEDBACK ss_is_pin_id_extended_timer(uint16_t pin_id, uint8_t *extended) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    switch(pin_id) {
        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9):
        case PIN('A', 8):
        case PIN('A', 9):
        case PIN('A', 10):
        case PIN('A', 11):
            *extended = 1;
            break;

        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 2):
        case PIN('A', 3):
        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1):
        case PIN('A', 5):
        case PIN('A', 15):
        case PIN('B', 10):
        case PIN('B', 11):
        case PIN('B', 14):
        case PIN('B', 15):
            *extended = 0;
            break;

        default: 
            rc = SS_FEEDBACK_PWM_PIN_ID_ERROR;
            break;
    }

    return rc;
}

SS_FEEDBACK ss_pwm_get_frequenzy_from_clock_config(uint16_t pin_id, uint16_t* frequency) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    
    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 2):
        case PIN('A', 3):
            *frequency = ss_clock.apb1;
            break; 

        case PIN('A', 5):
        case PIN('A', 15):
        case PIN('B', 10):
        case PIN('B', 11): 
            *frequency = ss_clock.apb1;
            break;

        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1):  
            *frequency = ss_clock.apb1;
            break;

        case PIN('A', 8):
        case PIN('A', 9):
        case PIN('A', 10):
        case PIN('A', 11): 
            *frequency = ss_clock.apb2;
            break;

        case PIN('B', 14):
        case PIN('B', 15): 
            *frequency = ss_clock.apb1;
            break;

        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9):  
            *frequency = ss_clock.apb2;
            break;

        default:
            rc = SS_FEEDBACK_PWM_PIN_ID_ERROR;
            break;
    }

    return rc;
}

SS_FEEDBACK ss_pwm_init(uint16_t pin_id, uint32_t frequency) {
    SS_FEEDBACK rc;

    rc = ss_io_init(pin_id, GPIO_MODE_AF);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint8_t af;
    rc = get_pwm_af_mode_for_pin_id(pin_id, &af);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    gpio_set_af(GPIO(PINBANK(pin_id)), af, BIT(PINNO(pin_id)));

    rc = ss_enable_timer_clock_from_pin_id(pin_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint32_t timer_id;
    rc = ss_get_timer_from_pin_id(pin_id, &timer_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint32_t channel;
    rc = ss_get_timer_channel_from_pin_id(pin_id, &channel);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint16_t fsys;
    rc = ss_pwm_get_frequenzy_from_clock_config(pin_id, &fsys);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    timer_set_mode(timer_id, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    timer_set_prescaler(timer_id, (fsys*1000000 / (100 * frequency)) - 1);

    timer_set_period(timer_id, 100);

    timer_set_oc_mode(timer_id, channel, TIM_OCM_PWM1);

    timer_enable_oc_preload(timer_id, channel);

    timer_set_oc_value(timer_id, channel, 0);

    timer_enable_counter(timer_id);

    uint8_t extended;
    rc = ss_is_pin_id_extended_timer(pin_id, &extended);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    if(extended) {
        timer_enable_break_main_output(timer_id);
    }
        
    timer_enable_oc_output(timer_id, channel);

    return SS_FEEDBACK_OK;
}

SS_FEEDBACK ss_pwm_init_highres(uint16_t pin_id, uint32_t frequency)  {
    SS_FEEDBACK rc;

    rc = ss_io_init(pin_id, GPIO_MODE_AF);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint8_t af;
    rc = get_pwm_af_mode_for_pin_id(pin_id, &af);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    gpio_set_af(GPIO(PINBANK(pin_id)), af, BIT(PINNO(pin_id)));

    rc = ss_enable_timer_clock_from_pin_id(pin_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint32_t timer_id; 
    rc = ss_get_timer_from_pin_id(pin_id, &timer_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint32_t channel;
    rc = ss_get_timer_channel_from_pin_id(pin_id, &channel);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint16_t fsys;
    rc = ss_pwm_get_frequenzy_from_clock_config(pin_id, &fsys);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    timer_set_mode(timer_id, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    timer_set_prescaler(timer_id, (fsys*1000000 / (100 * frequency)) - 1);

    timer_set_period(timer_id, 1000);

    timer_set_oc_mode(timer_id, channel, TIM_OCM_PWM1);

    timer_enable_oc_preload(timer_id, channel);

    timer_set_oc_value(timer_id, channel, 0);

    timer_enable_counter(timer_id);

    uint8_t extended;
    rc = ss_is_pin_id_extended_timer(pin_id, &extended);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    if(extended) {
        timer_enable_break_main_output(timer_id);
    }
        
    timer_enable_oc_output(timer_id, channel);

    return SS_FEEDBACK_OK;
}


SS_FEEDBACK ss_pwm_write(uint16_t pin_id, uint32_t value) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    uint32_t timer_id;
    rc = ss_get_timer_from_pin_id(pin_id, &timer_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint32_t channel;
    rc = ss_get_timer_channel_from_pin_id(pin_id, &channel);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    timer_set_oc_value(timer_id, channel, value);

    return SS_FEEDBACK_OK;
}

SS_FEEDBACK ss_pwm_write_highres(uint16_t pin_id, uint32_t value) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    uint32_t timer_id;
    rc = ss_get_timer_from_pin_id(pin_id, &timer_id);
    SS_HANDLE_ERROR_WITH_EXIT(timer_id);

    uint32_t channel;
    rc = ss_get_timer_channel_from_pin_id(pin_id, &channel);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    timer_set_oc_value(timer_id, channel, value);
    
    return SS_FEEDBACK_OK;
}