#include "ss_gpio.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "ss_delay.h"

int8_t ss_enable_rcc_from_id(uint16_t pin_id) {
    int8_t status = 0;
    switch (PINBANK(pin_id)) {
        case 0: rcc_periph_clock_enable(RCC_GPIOA); break;
        case 1: rcc_periph_clock_enable(RCC_GPIOB); break;
        case 2: rcc_periph_clock_enable(RCC_GPIOC); break;
        case 3: rcc_periph_clock_enable(RCC_GPIOD); break;
        default: status = -1; break;
    }
    return status;
}

uint16_t ss_io_init(uint16_t pin_id, uint8_t mode) {
    
    
    gpio_mode_setup(GPIO(PINBANK(pin_id)), mode, GPIO_PUPD_PULLDOWN, BIT(PINNO(pin_id)));

    gpio_clear(GPIO(PINBANK(pin_id)), BIT(PINNO(pin_id)));
    ss_enable_rcc_from_id(pin_id);

    return pin_id;
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