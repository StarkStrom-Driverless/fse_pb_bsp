#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/cm3/nvic.h>

#include <ss_gpio.h>
#include <ss_makros.h>

#include "ss_iob.h"

void ss_iob_init(struct IOB* iob) {
    for (int i = 0; i < MAX_INPUT_OBSERVATIONS; i++) {
        iob->iobs[i].enabled = 0;
    }
}

uint32_t get_exti_from_pin_id(uint16_t pin_id) {
    uint8_t id = PINNO(pin_id);
    uint32_t exti = 0;
    switch (id)
    {
        case 0: exti = EXTI0; break;
        case 1: exti = EXTI1; break;
        case 2: exti = EXTI2; break;
        case 3: exti = EXTI3; break;
        case 4: exti = EXTI4; break;
        case 5: exti = EXTI5; break;
        case 6: exti = EXTI6; break;
        case 7: exti = EXTI7; break;
        case 8: exti = EXTI8; break;
        case 9: exti = EXTI9; break;
        case 10: exti = EXTI10; break;
        case 11: exti = EXTI11; break;
        case 12: exti = EXTI12; break;
        case 13: exti = EXTI13; break;
        case 14: exti = EXTI14; break;
        case 15: exti = EXTI15; break;
    
        default: break;
    }

    return exti;
}

uint32_t get_port_from_pin_id(uint16_t pin_id) {
    uint8_t port = PINBANK(pin_id);
    uint32_t cm3_port = 0;
    switch(port) {
        case 0: cm3_port = GPIOA; break;
        case 1: cm3_port = GPIOB; break;
        case 2: cm3_port = GPIOC; break;
        case 3: cm3_port = GPIOD; break;
        default: break;
    }
    return cm3_port;
}

uint8_t get_nvic_exit_from_pin_id(uint16_t pin_id) {
    uint8_t nvic_exti = 0;

    switch(pin_id) {
        case 0: nvic_exti = NVIC_EXTI0_IRQ; break;
        case 1: nvic_exti = NVIC_EXTI1_IRQ; break;
        case 2: nvic_exti = NVIC_EXTI2_IRQ; break;
        case 3: nvic_exti = NVIC_EXTI3_IRQ; break;
        case 4: nvic_exti = NVIC_EXTI4_IRQ; break;

        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            nvic_exti = NVIC_EXTI9_5_IRQ ; break;

        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            nvic_exti = NVIC_EXTI15_10_IRQ ; break;

        default: break;
    }

    return nvic_exti;
}

uint16_t ss_iob_add(uint16_t pin_id, struct IOB* iob, uint8_t polarity) {
    ss_io_init(pin_id, SS_GPIO_MODE_INPUT);

    rcc_periph_clock_enable(RCC_SYSCFG);

    uint32_t exti = get_exti_from_pin_id(pin_id);
    uint32_t port = get_port_from_pin_id(pin_id);
    uint8_t nvic_exti = get_nvic_exit_from_pin_id(pin_id);

    iob->iobs[PINNO(pin_id)].enabled = 1;
    iob->iobs[PINNO(pin_id)].pin_id = pin_id;
    

    exti_select_source(exti, port);

    if (polarity == SS_GPIO_RAISING) {
        exti_set_trigger(exti, EXTI_TRIGGER_RISING);
        iob->iobs[PINNO(pin_id)].value = 0;
        iob->iobs[PINNO(pin_id)].polarity = SS_GPIO_RAISING;

    } else {
        exti_set_trigger(exti, EXTI_TRIGGER_FALLING);
        iob->iobs[PINNO(pin_id)].value = 1;
        iob->iobs[PINNO(pin_id)].polarity = SS_GPIO_FALLING;
    }

    exti_enable_request(exti);
    
    nvic_enable_irq(nvic_exti);

    return pin_id;
}

uint8_t exti_get_pending(uint8_t line) {
    if (line > 15) return false;
    return (EXTI_PR & (1 << line)) != 0;
}

uint8_t ss_iob_get(uint16_t pin_id, struct IOB* iob) {
    if (iob->iobs[PINNO(pin_id)].polarity == SS_GPIO_RAISING) {
        if (iob->iobs[PINNO(pin_id)].value) {
            iob->iobs[PINNO(pin_id)].value = 0;
            return 1;
        } else {
            if (ss_io_read(pin_id) == 1) {
                return 1;
            } else {
                return 0;
            }
        }
    } else {
        if (iob->iobs[PINNO(pin_id)].value) {
            iob->iobs[PINNO(pin_id)].value = 0;
            return 1;
        } else {
            if (ss_io_read(pin_id) == 0) {
                return 1;
            } else {
                return 0;
            }
        }
    }
}



void exti0_isr(void) {
    exti_reset_request(EXTI0);
    iob.iobs[0].value = 1;
}

void exti1_isr(void) {
    exti_reset_request(EXTI1);
    iob.iobs[1].value = 1;  
}

void exti2_isr(void) {
    exti_reset_request(EXTI2);
    iob.iobs[2].value = 1;  
}

void exti3_isr(void) {
    exti_reset_request(EXTI3);
    iob.iobs[3].value = 1;  
}

void exti4_isr(void) {
    exti_reset_request(EXTI4);
    iob.iobs[4].value = 1; 
}

void exti9_5_isr(void) {
    int8_t exti = -1;

    if (exti_get_pending(5)) {
        exti_reset_request(EXTI5);
        exti = 5;
    }
    else if (exti_get_pending(6)) {
        exti_reset_request(EXTI6);
        exti = 6;
    }
    else if (exti_get_pending(7)) {
        exti_reset_request(EXTI7);
        exti = 7;
    }
    else if (exti_get_pending(8)) {
        exti_reset_request(EXTI8);
        exti = 8;
    }
    else if (exti_get_pending(9)) {
        exti_reset_request(EXTI9);
        exti = 9;
    }

    if (exti != -1) {
        iob.iobs[exti].value = 1;
    }
    
}

void exti15_10_isr(void) {
    int8_t exti = -1;

    if (exti_get_pending(10)) {
        exti_reset_request(EXTI10);
        exti = 10;
    }
    if (exti_get_pending(11)) {
        exti_reset_request(EXTI11);
        exti = 11;
    }
    if (exti_get_pending(12)) {
        exti_reset_request(EXTI12);
        exti = 12;
    }
    if (exti_get_pending(13)) {
        exti_reset_request(EXTI13);
        exti = 13;
    }
    if (exti_get_pending(14)) {
        exti_reset_request(EXTI14);
        exti = 14;
    }
    if (exti_get_pending(15)) {
        exti_reset_request(EXTI15);
        exti = 15;
    }

    if (exti != -1) {
        iob.iobs[exti].value = 1;
    }
}