/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_gpio.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>



SS_FEEDBACK ss_enable_rcc_from_id(uint16_t pin_id) {

    switch (PINBANK(pin_id)) {
        case 0: rcc_periph_clock_enable(RCC_GPIOA); break;
        case 1: rcc_periph_clock_enable(RCC_GPIOB); break;
        case 2: rcc_periph_clock_enable(RCC_GPIOC); break;
        case 3: rcc_periph_clock_enable(RCC_GPIOD); break;
        default: 
            return SS_FEEDBACK_RCC_INIT_ERROR;
            break;
    }
    return SS_FEEDBACK_OK;
}

SS_FEEDBACK ss_io_init(uint16_t pin_id, uint8_t mode) {
    
    if (ss_enable_rcc_from_id(pin_id) !=  SS_FEEDBACK_OK) {
        return SS_SET_TOPLEVEL_ERROR(SS_FEEDBACK_IO_INIT_ERROR, SS_FEEDBACK_RCC_INIT_ERROR);
    }

    if (mode == SS_GPIO_MODE_INPUT_PU) {
        gpio_mode_setup(GPIO(PINBANK(pin_id)), SS_GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, BIT(PINNO(pin_id)));
    }
    else if (mode == SS_GPIO_MODE_INPUT_PD) {
        gpio_mode_setup(GPIO(PINBANK(pin_id)), SS_GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, BIT(PINNO(pin_id)));
    }
    else {
        gpio_mode_setup(GPIO(PINBANK(pin_id)), mode, GPIO_PUPD_NONE, BIT(PINNO(pin_id)));
    }
    
    return SS_FEEDBACK_OK;
}

void ss_io_write(uint16_t pin_id, uint8_t value) {
    if (value == SS_GPIO_TOGGLE) {
        gpio_toggle(GPIO(PINBANK(pin_id)), BIT(PINNO(pin_id)));
    } else {
        if (value == SS_GPIO_OFF) {
            gpio_clear(GPIO(PINBANK(pin_id)), BIT(PINNO(pin_id)));
        } else {
            gpio_set(GPIO(PINBANK(pin_id)), BIT(PINNO(pin_id)));
        }
    }
}

uint16_t ss_io_read(uint16_t pin_id) {
    return gpio_get(GPIO(PINBANK(pin_id)), BIT(PINNO(pin_id)));
}