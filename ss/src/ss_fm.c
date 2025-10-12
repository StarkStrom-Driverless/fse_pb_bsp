#include "ss_fm.h"
#include "ss_pwm.h"
#include "ss_gpio.h"
#include <stddef.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "ss_clock.h"

#define SS_FEEDBACK_BASE SS_FEEDBACK_FM_INIT_ERROR

struct SS_FREQ_MEASURE ss_fm;


/***
 * 
 * PERIPH FUNCTIONS
 * 
 */
uint32_t ss_fm_get_ic_from_pin_id(uint16_t pin_id) {
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

uint32_t ss_fm_get_iqr_cc_from_pin_id(uint16_t pin_id) {
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

uint32_t ss_fm_get_irq_from_pin_id(uint16_t pin_id) {
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

struct FREQ_PIN* ss_fm_get_pin_struct_from_pin_id(uint16_t pin_id) {
    struct FREQ_PIN* freq_pin_ptr = NULL;

    switch(pin_id) {
        case PIN('A', 0): freq_pin_ptr = &ss_fm.ports[3].pins[0]; break;
        case PIN('A', 1): freq_pin_ptr = &ss_fm.ports[3].pins[1]; break;
        case PIN('A', 2): freq_pin_ptr = &ss_fm.ports[3].pins[2]; break;
        case PIN('A', 3): freq_pin_ptr = &ss_fm.ports[3].pins[3]; break;

        case PIN('A', 5): freq_pin_ptr = &ss_fm.ports[1].pins[0]; break;
        case PIN('A', 15): freq_pin_ptr = &ss_fm.ports[1].pins[1]; break;
        case PIN('B', 10): freq_pin_ptr = &ss_fm.ports[1].pins[2]; break;
        case PIN('B', 11): freq_pin_ptr = &ss_fm.ports[1].pins[3]; break;

        case PIN('A', 6): freq_pin_ptr = &ss_fm.ports[2].pins[0]; break;
        case PIN('A', 7): freq_pin_ptr = &ss_fm.ports[2].pins[1]; break;
        case PIN('B', 0): freq_pin_ptr = &ss_fm.ports[2].pins[2]; break;
        case PIN('B', 1): freq_pin_ptr = &ss_fm.ports[2].pins[3]; break;

        case PIN('A', 8): freq_pin_ptr = &ss_fm.ports[0].pins[0]; break;
        case PIN('A', 9): freq_pin_ptr = &ss_fm.ports[0].pins[1]; break;
        case PIN('A', 10): freq_pin_ptr = &ss_fm.ports[0].pins[2]; break;
        case PIN('A', 11): freq_pin_ptr = &ss_fm.ports[0].pins[3]; break;

        case PIN('B', 14): freq_pin_ptr = &ss_fm.ports[5].pins[0]; break;
        case PIN('B', 15): freq_pin_ptr = &ss_fm.ports[5].pins[1]; break;

        case PIN('C', 6): freq_pin_ptr = &ss_fm.ports[4].pins[0]; break;
        case PIN('C', 7): freq_pin_ptr = &ss_fm.ports[4].pins[1]; break;
        case PIN('C', 8): freq_pin_ptr = &ss_fm.ports[4].pins[2]; break;
        case PIN('C', 9): freq_pin_ptr = &ss_fm.ports[4].pins[3]; break;

        default: break;
    }

    return freq_pin_ptr;
}



uint8_t ss_fm_get_af_from_pin_id(uint16_t pin_id) {
    uint8_t af = 0;

    switch (pin_id) {

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

uint32_t ss_fm_get_ti_from_pin_id(uint16_t pin_id) {
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


/***
 * 
 * USER FUNCTIONS
 * 
 */
SS_FEEDBACK ss_fm_init(uint16_t pin_id,  uint32_t resolution) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    uint32_t clock_frequency = 0;
    
    ss_io_init(pin_id, SS_GPIO_MODE_AF);
    gpio_set_af(GPIO(PINBANK(pin_id)), ss_fm_get_af_from_pin_id(pin_id), BIT(PINNO(pin_id)));
    
    uint32_t timer = ss_get_timer_from_pin_id(pin_id);
    rc = ss_clock_fm(pin_id, &clock_frequency);

    uint32_t ic = ss_fm_get_ic_from_pin_id(pin_id);
    SS_HANDLE_NULL_WITH_EXIT(ic);

    uint32_t irq = ss_fm_get_iqr_cc_from_pin_id(pin_id);
    SS_HANDLE_NULL_WITH_EXIT(irq);

    uint32_t nvic_irq = ss_fm_get_irq_from_pin_id(pin_id);
    SS_HANDLE_NULL_WITH_EXIT(nvic_irq);

    uint32_t ti = ss_fm_get_ti_from_pin_id(pin_id);
    SS_HANDLE_NULL_WITH_EXIT(ti);

    uint32_t prescaler = (clock_frequency * 1000000 / resolution) - 1;

    struct FREQ_PIN* channel = ss_fm_get_pin_struct_from_pin_id(pin_id);
    SS_HANDLE_NULL_WITH_EXIT(channel);


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


    return rc;
}

SS_FEEDBACK ss_fm_read(uint16_t pin_id, float *value) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    struct FREQ_PIN* channel = ss_fm_get_pin_struct_from_pin_id(pin_id);
    SS_HANDLE_NULL_WITH_EXIT(channel);

    if (!channel->enabled) {
        rc = SS_FEEDBACK_FM_PIN_NOT_ENABLED;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    
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
            channel->frequency = 0.0f;
        }

    }

    if (channel->requested < 100) {
        channel->requested++;
    }
    
    *value = channel->frequency;

    return rc;
}


/***
 * 
 * ISR FUNCTIONS
 * 
 */
void ss_fm_isr(uint32_t timer, uint8_t port_id) {
    for (uint8_t i = 0; i < 4; i++) {
        struct FREQ_PIN* channel = &ss_fm.ports[port_id].pins[i];

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
    ss_fm_isr(TIM2, 1);
}

void tim1_cc_isr(void) {
    ss_fm_isr(TIM1, 0);
}