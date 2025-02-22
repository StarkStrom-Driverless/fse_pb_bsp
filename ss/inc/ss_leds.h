#ifndef _SS_LEDS_H_
#define _SS_LEDS_H_


int8_t ss_leds_init(void);

void ss_led_error_on(void);
void ss_led_error_off(void);
void ss_led_error_toggle(void);

void ss_led_heartbeat_on(void);
void ss_led_heartbeat_off(void);
void ss_led_heartbeat_toggle(void);

void ss_led_dbg1_on(void);
void ss_led_dbg1_off(void);
void ss_led_dbg1_toggle(void);

void ss_led_dbg2_on(void);
void ss_led_dbg2_off(void);
void ss_led_dbg2_toggle(void);


#endif