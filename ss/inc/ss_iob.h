/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_IOB

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

SS_FEEDBACK ss_iob_add(uint16_t pin_id, uint8_t polarity);

#ifdef USE_PRIVATE
SS_FEEDBACK get_port_from_pin_id(uint16_t pin_id, uint32_t* cm3_port);
#endif
#ifdef USE_PRIVATE
SS_FEEDBACK get_nvic_exit_from_pin_id(uint16_t pin_id, uint32_t* nvic_exti);
#endif
#ifdef USE_PRIVATE
SS_FEEDBACK get_exti_from_pin_id(uint16_t pin_id, uint32_t* exti);
#endif

uint8_t ss_iob_get(uint16_t pin_id);

#ifdef USE_PRIVATE
uint8_t exti_get_pending(uint8_t line);
#endif

#ifdef USE_PRIVATE
void exti0_isr(void);
#endif
#ifdef USE_PRIVATE
void exti1_isr(void);
#endif
#ifdef USE_PRIVATE
void exti2_isr(void);
#endif
#ifdef USE_PRIVATE
void exti3_isr(void);
#endif
#ifdef USE_PRIVATE
void exti4_isr(void);
#endif
#ifdef USE_PRIVATE
void exti9_5_isr(void);
#endif
#ifdef USE_PRIVATE
void exti15_10_isr(void);
#endif

#endif // _SS_IOB_H_

#endif // COMPILE_SS_IOB
