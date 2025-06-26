#include "ss_freq_measure.h"
#include "ss_pwm.h"
#include "ss_clock.h"
#include <libopencm3/cm3/nvic.h>

struct SS_FREQ_MEASURE ss_freq_measure;

uint16_t ss_freq_get_bus_freq_from_pin_id(uint16_t pin_id, struct SS_CLOCK* clock) {
    uint16_t frequency = 0;

    switch (pin_id) {
        case PIN('A', 8):
        case PIN('A', 9):
        case PIN('A', 10):
        case PIN('A', 11): 
        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9): 

            frequency = clock->apb2;
            break;

        case PIN('A', 5):
        case PIN('A', 15):
        case PIN('B', 10):
        case PIN('B', 11): 
        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1): 
        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 2):
        case PIN('A', 3):
        case PIN('B', 14):
        case PIN('B', 15): 
            frequency = clock->apb1;
            break;

        default: break;
    }

    return frequency;
}

uint32_t ss_freq_get_ic_from_pin_id(uint16_t pin_id) {
    uint32_t ic_channel = 0;
    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 5):
        case PIN('A', 6):
        case PIN('A', 8):
        case PIN('A', 15):
        case PIN('B', 14):
        case PIN('C', 6):  
            ic_channel = TIM_IC1; 
            break;

        case PIN('A', 2):
        case PIN('A', 10):
        case PIN('B', 0):
        case PIN('B', 10):
        case PIN('C', 8):  
            ic_channel = TIM_IC3; 
            break;

        case PIN('A', 3):
        case PIN('A', 11):
        case PIN('B', 1):
        case PIN('B', 11):
        case PIN('C', 9):
            ic_channel = TIM_IC4; 
            break;

        case PIN('A', 1):
        case PIN('A', 7):
        case PIN('A', 9):
        case PIN('B', 15):
        case PIN('C', 7):  
            ic_channel = TIM_IC2; 
            break;

        default: break;
    }

    return ic_channel;
}

uint32_t ss_freq_get_iqr_from_pin_id(uint16_t pin_id) {
    uint32_t irq = 0;

    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 5):
        case PIN('A', 6):
        case PIN('A', 8):
        case PIN('A', 15):
        case PIN('B', 14):
        case PIN('C', 6):  
            irq = TIM_DIER_CC1IE; 
            break;

        case PIN('A', 2):
        case PIN('A', 10):
        case PIN('B', 0):
        case PIN('B', 10):
        case PIN('C', 8):  
            irq = TIM_DIER_CC3IE; 
            break;

        case PIN('A', 3):
        case PIN('A', 11):
        case PIN('B', 1):
        case PIN('B', 11):
        case PIN('C', 9):
            irq = TIM_DIER_CC4IE; 
            break;

        case PIN('A', 1):
        case PIN('A', 7):
        case PIN('A', 9):
        case PIN('B', 15):
        case PIN('C', 7):  
            irq = TIM_DIER_CC2IE; 
            break;

        default: break;
    }

    return irq;
}

uint32_t ss_freq_get_nvic_irq_from_pin_id(uint16_t pin_id) {
    uint32_t irq = 0;

    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 2):
        case PIN('A', 3):
            irq = NVIC_TIM5_IRQ; 
            break; 

        case PIN('A', 5):
        case PIN('A', 15):
        case PIN('B', 10):
        case PIN('B', 11): 
            irq = NVIC_TIM2_IRQ; 
            break;

        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1):  
            irq = NVIC_TIM3_IRQ; 
            break;

        case PIN('A', 8):
        case PIN('A', 9):
        case PIN('A', 10):
        case PIN('A', 11): 
            irq = NVIC_TIM1_IRQ; 
            break;

        case PIN('B', 14):
        case PIN('B', 15): 
            irq = NVIC_TIM12_IRQ; 
            break;

        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9):  
            irq = NVIC_TIM8_IRQ; 
            break;

        default: break;
    }
}

struct FREQ_CHANNEL* ss_freq_get_channel_from_pin_id(uint16_t pin_id) {
    struct FREQ_CHANNEL* channel = NULL;
    
    if (pin_id <= 0b00111111) {
        channel = &ss_freq_measure.freq_block[pin_id];
    }

    return channel;
}

struct FREQ_CHANNEL* ss_freq_measure_init(uint16_t pin_id, struct SS_CLOCK* clock, uint32_t resolution) {
    uint32_t timer = ss_get_timer_from_pin_id(pin_id);
    uint16_t clock_frequency = ss_freq_get_bus_freq_from_pin_id(pin_id, clock);
    uint32_t ic = ss_freq_get_ic_from_pin_id(pin_id);
    uint32_t irq = ss_freq_get_iqr_from_pin_id(pin_id);
    uint32_t nvic_irq = ss_freq_get_nvic_irq_from_pin_id(pin_id);

    struct FREQ_CHANNEL* channel = ss_freq_get_channel_from_pin_id(pin_id);

    if (channel != NULL) {
        ss_enable_timer_clock_from_pin_id(pin_id);

        timer_disable_counter(timer);
    
        timer_set_prescaler(timer, clock_frequency * 1000000 / resolution - 1);
    
        timer_set_period(timer, 0xFFFFFFFF);
    
        timer_ic_set_input(timer, ic, TIM_IC_IN_TI1);
    
        timer_ic_set_polarity(timer, ic, TIM_IC_RISING);
    
        timer_ic_set_prescaler(timer, ic, TIM_IC_PSC_OFF);
    
        timer_ic_set_filter(timer, ic, TIM_IC_OFF);
    
    
        timer_enable_irq(timer, irq);
    
        nvic_enable_irq(nvic_irq);
    
    
        timer_enable_counter(timer);


        channel->frequency = 0;
        channel->last_capture = 0;
        channel->resolution = resolution;
    }

    return channel;
}

void tim2_isr(void) {
    struct FREQ_CHANNEL* channel = 
    if (TIM_SR(TIM2) & TIM_SR_CC1IF) {
        ss_freq_get_channel_from_pin_id(PIN())
        uint32_t now = TIM_CCR1(TIM2);
        uint32_t diff = (now >= last_capture) ? (now - last_capture) : (0xFFFFFFFF - last_capture + now + 1);
        last_capture = now;

        if (diff != 0) {
            frequency_hz = 1000000 / diff;
        }

        TIM_SR(TIM2) &= ~TIM_SR_CC1IF;
    }

}