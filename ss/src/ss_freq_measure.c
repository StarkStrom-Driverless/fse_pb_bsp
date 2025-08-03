#include "ss_freq_measure.h"
#include "ss_pwm.h"
#include "ss_gpio.h"
#include <stddef.h>
#include <libopencm3/stm32/timer.h>
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

            frequency = clock->apb2 * 2;
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
            frequency = clock->apb1 * 2;
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
            irq = NVIC_TIM1_CC_IRQ; 
            break;

        case PIN('B', 14):
        case PIN('B', 15): 
            irq = NVIC_TIM8_BRK_TIM12_IRQ; 
            break;

        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9):  
            irq = NVIC_TIM8_CC_IRQ; 
            break;

        default: break;
    }

    return irq;
}

struct FREQ_PIN* ss_get_freq_pin_from_pin_id(uint16_t pin_id) {
    struct FREQ_PIN* freq_pin_ptr = NULL;

    switch(pin_id) {
        case PIN('A', 0): freq_pin_ptr = &ss_freq_measure.ports[3].pins[0]; break;
        case PIN('A', 1): freq_pin_ptr = &ss_freq_measure.ports[3].pins[1]; break;
        case PIN('A', 2): freq_pin_ptr = &ss_freq_measure.ports[3].pins[2]; break;
        case PIN('A', 3): freq_pin_ptr = &ss_freq_measure.ports[3].pins[3]; break;

        case PIN('A', 5): freq_pin_ptr = &ss_freq_measure.ports[1].pins[0]; break;
        case PIN('A', 15): freq_pin_ptr = &ss_freq_measure.ports[1].pins[1]; break;
        case PIN('B', 10): freq_pin_ptr = &ss_freq_measure.ports[1].pins[2]; break;
        case PIN('B', 11): freq_pin_ptr = &ss_freq_measure.ports[1].pins[3]; break;

        case PIN('A', 6): freq_pin_ptr = &ss_freq_measure.ports[2].pins[0]; break;
        case PIN('A', 7): freq_pin_ptr = &ss_freq_measure.ports[2].pins[1]; break;
        case PIN('B', 0): freq_pin_ptr = &ss_freq_measure.ports[2].pins[2]; break;
        case PIN('B', 1): freq_pin_ptr = &ss_freq_measure.ports[2].pins[3]; break;

        case PIN('A', 8): freq_pin_ptr = &ss_freq_measure.ports[0].pins[0]; break;
        case PIN('A', 9): freq_pin_ptr = &ss_freq_measure.ports[0].pins[1]; break;
        case PIN('A', 10): freq_pin_ptr = &ss_freq_measure.ports[0].pins[2]; break;
        case PIN('A', 11): freq_pin_ptr = &ss_freq_measure.ports[0].pins[3]; break;

        case PIN('B', 14): freq_pin_ptr = &ss_freq_measure.ports[5].pins[0]; break;
        case PIN('B', 15): freq_pin_ptr = &ss_freq_measure.ports[5].pins[1]; break;

        case PIN('C', 6): freq_pin_ptr = &ss_freq_measure.ports[4].pins[0]; break;
        case PIN('C', 7): freq_pin_ptr = &ss_freq_measure.ports[4].pins[1]; break;
        case PIN('C', 8): freq_pin_ptr = &ss_freq_measure.ports[4].pins[2]; break;
        case PIN('C', 9): freq_pin_ptr = &ss_freq_measure.ports[4].pins[3]; break;

        default: break;
    }

    return freq_pin_ptr;
}



uint8_t get_freq_af_mode_for_pin_id(uint16_t pin_id) {
    uint8_t af = 0;

    switch (pin_id) {
        // TIM2 / TIM3 / TIM5 → AF2
        case PIN('A', 0): // TIM5_CH1
        case PIN('A', 1): // TIM5_CH2
        case PIN('A', 2): // TIM5_CH3
        case PIN('A', 3): // TIM5_CH4
        case PIN('A', 6): // TIM3_CH1
        case PIN('A', 7): // TIM3_CH2
        case PIN('B', 0): // TIM3_CH3
        case PIN('B', 1): // TIM3_CH4
            af = 2;
            break;

        // TIM1 / TIM2 → AF1
        case PIN('A', 5):  // TIM2_CH1
        case PIN('A', 8):  // TIM1_CH1
        case PIN('A', 9):  // TIM1_CH2
        case PIN('A', 10): // TIM1_CH3
        case PIN('A', 11): // TIM1_CH4
        case PIN('A', 15): // TIM2_CH1
        case PIN('B', 10): // TIM2_CH3
        case PIN('B', 11): // TIM2_CH4
            af = 1;
            break;

        // TIM8 / TIM12 → AF9
        case PIN('B', 14): // TIM12_CH1
        case PIN('B', 15): // TIM12_CH2
            af = 9;
            break;

        // TIM8 → AF3
        case PIN('C', 6): // TIM8_CH1
        case PIN('C', 7): // TIM8_CH2
        case PIN('C', 8): // TIM8_CH3
        case PIN('C', 9): // TIM8_CH4
            af = 3;
            break;

        default: break;
    }

    return af;
}

uint32_t ss_freq_get_ti_from_pin_id(uint16_t pin_id) {
    uint32_t ti = 0;

    switch(pin_id) {
        case PIN('A', 0):
        case PIN('A', 5):
        case PIN('A', 6):
        case PIN('A', 8):
        case PIN('A', 15):
        case PIN('B', 14):
        case PIN('C', 6):  
            ti = TIM_IC_IN_TI1; 
            break;

        case PIN('A', 2):
        case PIN('A', 10):
        case PIN('B', 0):
        case PIN('B', 10):
        case PIN('C', 8):  
            ti = TIM_IC_IN_TI3; 
            break;

        case PIN('A', 3):
        case PIN('A', 11):
        case PIN('B', 1):
        case PIN('B', 11):
        case PIN('C', 9):
            ti = TIM_IC_IN_TI4; 
            break;

        case PIN('A', 1):
        case PIN('A', 7):
        case PIN('A', 9):
        case PIN('B', 15):
        case PIN('C', 7):  
            ti = TIM_IC_IN_TI2; 
            break;

        default: break;
    }

    return ti;
}

struct FREQ_PIN* ss_freq_measure_init(uint16_t pin_id, struct SS_CLOCK* clock, uint32_t resolution) {
    ss_io_init(pin_id, SS_GPIO_MODE_AF);

    gpio_set_af(GPIO(PINBANK(pin_id)), get_freq_af_mode_for_pin_id(pin_id), BIT(PINNO(pin_id)));
    
    uint32_t timer = ss_get_timer_from_pin_id(pin_id);
    uint16_t clock_frequency = ss_freq_get_bus_freq_from_pin_id(pin_id, clock);
    uint32_t ic = ss_freq_get_ic_from_pin_id(pin_id);
    uint32_t irq = ss_freq_get_iqr_from_pin_id(pin_id);
    uint32_t nvic_irq = ss_freq_get_nvic_irq_from_pin_id(pin_id);
    uint32_t ti = ss_freq_get_ti_from_pin_id(pin_id);

    uint32_t prescaler = (clock_frequency * 1000000 / resolution) - 1;

    struct FREQ_PIN* channel = ss_get_freq_pin_from_pin_id(pin_id);

    if (channel != NULL) {
        ss_enable_timer_clock_from_pin_id(pin_id);

        timer_disable_counter(timer);
    
        timer_set_prescaler(timer, prescaler);
        timer_set_period(timer, 0xFFFF);

        
    
        timer_ic_set_input(timer, ic, ti);
        timer_ic_set_polarity(timer, ic, TIM_IC_RISING);
        timer_ic_set_prescaler(timer, ic, TIM_IC_PSC_OFF);
        timer_ic_set_filter(timer, ic, TIM_IC_OFF);
        timer_ic_enable(timer, ic);

        
        timer_enable_irq(timer, irq);
        nvic_enable_irq(nvic_irq);
    
        timer_enable_counter(timer);

        switch (ic)
        {
            case TIM_IC1: channel->last_capture = TIM_CCR1(timer); break;
            case TIM_IC2: channel->last_capture = TIM_CCR2(timer); break;
            case TIM_IC3: channel->last_capture = TIM_CCR3(timer); break;
            case TIM_IC4: channel->last_capture = TIM_CCR4(timer); break;
            default: break;
        }


        channel->resolution = resolution;
        channel->pin_id = pin_id;
        channel->enabled = 1;
        channel->requested = 0;
        channel->frequency = 0.0f;
        channel->init = 0;
        channel->timer = timer;
        channel->ic = ic;
    }

    return channel;
}

float ss_freq_measure_get(uint16_t pin_id) {
    struct FREQ_PIN* channel = ss_get_freq_pin_from_pin_id(pin_id);

    if (!channel->enabled) return 0;

    
    if (channel->requested >= 15) {
        if (channel->requested < 30) {
            uint32_t timer = channel->timer;
            uint32_t ic = channel->ic;
            uint32_t now = 0;
            uint32_t diff = 0;

            switch (ic)
            {
                case TIM_IC1: now = TIM_CCR1(timer); break;
                case TIM_IC2: now = TIM_CCR2(timer); break;
                case TIM_IC3: now = TIM_CCR3(timer); break;
                case TIM_IC4: now = TIM_CCR4(timer); break;
                default: break;
            }

        
            diff = now - channel->last_capture;
            if (diff >= 0) {
                float freq = channel->resolution / diff;
                channel->frequency = freq;
            }
        } else {
            channel->frequency = 0;
        }

    }


    channel->requested++;

    return channel->frequency;
}

void ss_freq_meausure_isr(uint32_t timer, uint8_t port_id) {
    for (uint8_t i = 0; i < 4; i++) {
        struct FREQ_PIN* channel = &ss_freq_measure.ports[port_id].pins[i];

        if (!channel->enabled) continue;
        
        uint8_t active = 0;
        uint32_t now = 0;

        switch (i) {
            case 0: 
                if (TIM_SR(timer) & TIM_SR_CC1IF) {
                    active = 1;
                    now = TIM_CCR1(timer);
                    TIM_SR(timer) &= ~TIM_SR_CC1IF;
                }  
            break;

            case 1:
                if (TIM_SR(timer) & TIM_SR_CC2IF) {
                    active = 1;
                    now = TIM_CCR2(timer);
                    TIM_SR(timer) &= ~TIM_SR_CC2IF;
                }
            break;

            case 2:
                if (TIM_SR(timer) & TIM_SR_CC3IF) {
                    active = 1;
                    now = TIM_CCR3(timer);
                    TIM_SR(timer) &= ~TIM_SR_CC3IF;
                }
            break;

            case 3:
                if (TIM_SR(timer) & TIM_SR_CC4IF) {
                    active = 1;
                    now = TIM_CCR4(timer);
                    TIM_SR(timer) &= ~TIM_SR_CC4IF;
                }
            break;
        
            default: break;
        }

        if (!active) continue;

        uint32_t delta = 0;

        if (now >= channel->last_capture) {
            delta = now - channel->last_capture;
        } else {
            delta = (0xFFFF - channel->last_capture) + now;
        }

        channel->last_capture = now;

        float tmp = channel->resolution / delta;
        if (tmp < 5000.0f) {
            channel->frequency = channel->resolution / delta;
        }
        



        channel->requested = 0;
    }
}

void tim2_isr(void) {
    ss_freq_meausure_isr(TIM2, 1);
}

void tim1_cc_isr(void) {
    ss_freq_meausure_isr(TIM1, 0);
}