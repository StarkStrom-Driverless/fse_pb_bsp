#include "ss.h"

#include "pins.h"

uint16_t pwm_pin = EXAMPLE_PWM_PIN;

void example_task(void* args) {
    uint8_t value = 50;
    uint8_t direction = 0;
    uint8_t tick = 0;

    for (;;) {
        ss_pwm_write(pwm_pin, value);

        if (tick == 0) {
            ss_printf(4, "PWM: %d \r\n", value);
        }
        tick = (tick + 1) % 20;

        if (direction == 0) {
            if (value < 100) {
                value++;
            } else {
                direction = 1;
            }
        } else {
            if (value > 0) {
                value--;
            } else {
                direction = 0;
            }
        }

        ss_rtos_delay_ms(10);
    }
}

int main(void) {
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_uart_init(4, 115200));
    SS_ERROR_ASSERT(ss_pwm_init(pwm_pin, 10000));

    ss_printf(4, "fse_pb_bsp example: pwm \r\n");

    SS_ERROR_ASSERT(ss_rtos_task_add(example_task, NULL, 1, "example_task"));

    ss_rtos_start();

    while (1) {
    }

    return 0;
}

void ss_error_fail(void) {
    while (1) {
        ss_led_error_toggle();
        ss_delay(1000);
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName) {
    while(1) {
        ss_led_error_toggle();
        ss_delay(500);
    }
}
