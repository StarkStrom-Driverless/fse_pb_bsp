#ifndef _SS_PWM_H_
#define _SS_PWM_H_

#include <inttypes.h>
#include "ss_makros.h"
#include "ss_clock.h"
#include "ss_feedback.h"


SS_FEEDBACK ss_get_timer_channel_from_pin_id(uint16_t pin_id, uint32_t* timer_addr);
SS_FEEDBACK ss_get_timer_from_pin_id(uint16_t pin_id, uint32_t* timer_addr);
SS_FEEDBACK ss_enable_timer_clock_from_pin_id(uint16_t pin_id);
SS_FEEDBACK get_pwm_af_mode_for_pin_id(uint16_t pin_id, uint8_t* af);
SS_FEEDBACK ss_is_pin_id_extended_timer(uint16_t pin_id, uint8_t *extended);
SS_FEEDBACK ss_pwm_get_frequenzy_from_clock_config(uint16_t pin_id, uint16_t* frequency);
SS_FEEDBACK ss_pwm_init(uint16_t pin_id, uint32_t frequency);
SS_FEEDBACK ss_pwm_init_highres(uint16_t pin_id, uint32_t frequency);
SS_FEEDBACK ss_pwm_write(uint16_t pin_id, uint32_t value);
SS_FEEDBACK ss_pwm_write_highres(uint16_t pin_id, uint32_t value);

#endif