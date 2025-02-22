#include "ss_gpio.h"
#include "ss_leds.h"

int8_t ss_leds_init(void) {
    ss_io_init(PIN('C', 1), SS_GPIO_MODE_OUTPUT);
    ss_io_init(PIN('C', 0), SS_GPIO_MODE_OUTPUT);
    ss_io_init(PIN('C', 4), SS_GPIO_MODE_OUTPUT);
    ss_io_init(PIN('C', 5), SS_GPIO_MODE_OUTPUT);

    ss_led_error_off();
    ss_led_heartbeat_off();
    ss_led_dbg1_off();
    ss_led_dbg2_off();

    return 0;
}



void ss_led_error_on(void) {
    ss_io_write(PIN('C', 0), SS_GPIO_OFF);
}

void ss_led_error_off(void) {
    ss_io_write(PIN('C', 0), SS_GPIO_ON);
}

void ss_led_error_toggle(void) {
    ss_io_write(PIN('C', 0), SS_GPIO_TOGGLE);
}



void ss_led_heartbeat_on(void) {
    ss_io_write(PIN('C',1), SS_GPIO_OFF);
}

void ss_led_heartbeat_off(void) {
    ss_io_write(PIN('C',1), SS_GPIO_ON);
}

void ss_led_heartbeat_toggle(void) {
    ss_io_write(PIN('C',1), SS_GPIO_TOGGLE);
}



void ss_led_dbg1_on(void) {
    ss_io_write(PIN('C',4), SS_GPIO_OFF);
}

void ss_led_dbg1_off(void) {
    ss_io_write(PIN('C',4), SS_GPIO_ON);
}

void ss_led_dbg1_toggle(void) {
    ss_io_write(PIN('C',4), SS_GPIO_TOGGLE);
}



void ss_led_dbg2_on(void) {
    ss_io_write(PIN('C',5), SS_GPIO_OFF);
}

void ss_led_dbg2_off(void) {
    ss_io_write(PIN('C',5), SS_GPIO_ON);
}

void ss_led_dbg2_toggle(void) {
    ss_io_write(PIN('C',5), SS_GPIO_TOGGLE);
}