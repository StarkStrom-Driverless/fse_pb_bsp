#ifndef __SS_GPIO_H__
#define __SS_GPIO_H__

#include <inttypes.h>

#include "ss_makros.h"

uint32_t ss_get_timer_from_pin_id(uint16_t pin_id);

int8_t ss_enable_timer_clock_from_pin_id(uint16_t pin_id);

uint16_t ss_pwm_init(uint16_t pin_id, uint32_t frequency, uint32_t fsys);

uint8_t ss_pwm_write(uint16_t pin_id, uint32_t value);

uint8_t ss_is_pin_id_extended_timer(uint16_t pin_id);

uint16_t ss_pwm_init_highres(uint16_t pin_id);

uint8_t ss_pwm_write_highres(uint16_t pin_id, uint32_t value);

#endif