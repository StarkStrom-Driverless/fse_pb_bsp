#ifndef _SS_PWM_H_
#define _SS_PWM_H_

#include <inttypes.h>

#include "ss_makros.h"

#include "ss_clock.h"

uint16_t ss_pwm_get_frequenzy_from_clock_config(uint16_t pin_id, struct SS_CLOCK* clock);

uint32_t ss_get_timer_from_pin_id(uint16_t pin_id);

int8_t ss_enable_timer_clock_from_pin_id(uint16_t pin_id);

uint16_t ss_pwm_init(uint16_t pin_id, uint32_t frequency, struct SS_CLOCK* clock);

uint8_t ss_pwm_write(uint16_t pin_id, uint32_t value);

uint8_t ss_is_pin_id_extended_timer(uint16_t pin_id);

uint16_t ss_pwm_init_highres(uint16_t pin_id, uint32_t frequency, struct SS_CLOCK* clock);

uint8_t ss_pwm_write_highres(uint16_t pin_id, uint32_t value);

#endif