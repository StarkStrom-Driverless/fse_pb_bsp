#ifndef _SS_IOB_H_
#define _SS_IOB_H_

#include "ss_feedback.h"

#define MAX_INPUT_OBSERVATIONS 16

#define SS_GPIO_RAISING 0
#define SS_GPIO_FALLING 1

struct InputObservation {
    uint8_t value;
    uint8_t enabled;
    uint16_t pin_id;
    uint8_t polarity;
};

struct IOB {
    struct InputObservation iobs[MAX_INPUT_OBSERVATIONS];
};

extern struct IOB ss_iob;

SS_FEEDBACK ss_iob_init();

SS_FEEDBACK ss_iob_add(uint16_t pin_id, uint8_t polarity);

uint8_t ss_iob_get(uint16_t pin_id);

uint32_t get_exti_from_pin_id(uint16_t pin_id);

uint8_t exti_get_pending(uint8_t line);


void exti0_isr(void);
void exti1_isr(void);
void exti2_isr(void);
void exti3_isr(void);
void exti4_isr(void);
void exti9_5_isr(void);
void exti15_10_isr(void);

#endif