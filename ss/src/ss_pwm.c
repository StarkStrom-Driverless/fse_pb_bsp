#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include "ss_gpio.h"
#include "ss_pwm.h"

uint32_t ss_get_timer_channel_from_pin_id(uint16_t pin_id) {
    uint32_t timer_addr = 0;
    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 5):
        case PIN('A', 6):
        case PIN('A', 8):
        case PIN('A', 15):
        case PIN('B', 14):
        case PIN('C', 6):  
            timer_addr = TIM_OC1; 
            break;

        case PIN('A', 2):
        case PIN('A', 10):
        case PIN('B', 0):
        case PIN('B', 10):
        case PIN('C', 8):  
            timer_addr = TIM_OC3; 
            break;

        case PIN('A', 3):
        case PIN('A', 11):
        case PIN('B', 1):
        case PIN('B', 11):
        case PIN('C', 9):
            timer_addr = TIM_OC4; 
            break;

        case PIN('A', 1):
        case PIN('A', 7):
        case PIN('A', 9):
        case PIN('B', 15):
        case PIN('C', 7):  
            timer_addr = TIM_OC2; 
            break;

        default: break;
    }

    return timer_addr;
}

uint32_t ss_get_timer_from_pin_id(uint16_t pin_id) {
    uint32_t timer_addr = 0;
    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 2):
        case PIN('A', 3):
            timer_addr = TIM5; 
            break; 

        case PIN('A', 5):
        case PIN('A', 15):
        case PIN('B', 10):
        case PIN('B', 11): 
            timer_addr = TIM2; 
            break;

        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1):  
            timer_addr = TIM3; 
            break;

        case PIN('A', 8):
        case PIN('A', 9):
        case PIN('A', 10):
        case PIN('A', 11): 
            timer_addr = TIM1; 
            break;

        case PIN('B', 14):
        case PIN('B', 15): 
            timer_addr = TIM12; 
            break;

        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9):  
            timer_addr = TIM8; break;

        default: break;
    }

    return timer_addr;
}

int8_t ss_enable_timer_clock_from_pin_id(uint16_t pin_id) {
    uint32_t status = -1;
    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 2):
        case PIN('A', 3):
            rcc_periph_clock_enable(RCC_TIM5);
            status = 0;
            break;

        case PIN('A', 5):
        case PIN('A', 15):
        case PIN('B', 10):
        case PIN('B', 11):
            rcc_periph_clock_enable(RCC_TIM2);
            status = 0;
            break;

        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1):
            rcc_periph_clock_enable(RCC_TIM3);
            status = 0;
            break;

        case PIN('A', 8):
        case PIN('A', 9):
        case PIN('A', 10):
        case PIN('A', 11):
            rcc_periph_clock_enable(RCC_TIM1);
            status = 0;
            break;

        case PIN('B', 14):
        case PIN('B', 15):
            rcc_periph_clock_enable(RCC_TIM12);
            status = 0;
            break;

        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9):
            rcc_periph_clock_enable(RCC_TIM8);
            status = 0;
            break;

        default: break;
    }
    return status;
}

uint8_t get_pwm_af_mode_for_pin_id(uint16_t pin_id) {
    uint8_t af = 0;
    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 2):
        case PIN('A', 3):
        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1):
            af = 2;
            break;

        case PIN('A', 5):
        case PIN('A', 8):
        case PIN('A', 9):
        case PIN('A', 10):
        case PIN('A', 11):
        case PIN('A', 15):
        case PIN('B', 10):
        case PIN('B', 11):
            af = 1;
            break;

        case PIN('B', 14):
        case PIN('B', 15):
            af = 9;
            break;

        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9):
            af = 3;
            break;

        default: break;
    }
    return af;
}

uint16_t ss_pwm_init(uint16_t pin_id, uint32_t frequency, uint32_t fsys) {
    ss_io_init(pin_id, GPIO_MODE_AF);

    gpio_set_af(GPIO(PINBANK(pin_id)), get_pwm_af_mode_for_pin_id(pin_id), BIT(PINNO(pin_id)));

    ss_enable_timer_clock_from_pin_id(pin_id);

    uint32_t timer_id = ss_get_timer_from_pin_id(pin_id);
    uint8_t channel = ss_get_timer_channel_from_pin_id(pin_id);

    timer_set_mode(timer_id, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    timer_set_prescaler(timer_id, (fsys / (100 * frequency)) - 1);

    timer_set_period(timer_id, 100);

    timer_set_oc_mode(timer_id, channel, TIM_OCM_PWM1);

    timer_enable_oc_preload(timer_id, channel);

    timer_set_oc_value(timer_id, channel, 50);

    timer_enable_counter(timer_id);

    timer_enable_oc_output(timer_id, channel);

    return pin_id;
}

uint8_t ss_pwm_write(uint16_t pin_id, uint32_t value) {
    uint32_t timer_id = ss_get_timer_from_pin_id(pin_id);
    uint8_t channel = ss_get_timer_channel_from_pin_id(pin_id);
    timer_set_oc_value(timer_id, channel, value);
    return 0;
}