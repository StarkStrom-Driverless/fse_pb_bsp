#include "ss.h"

#include "pins.h"

uint16_t fm_pin = EXAMPLE_FM_PIN;

void example_task(void* args) {
    float value = 0.0f;
    uint8_t tick = 0;

    for (;;) {
        ss_fm_read(fm_pin, &value);

        ss_led_heartbeat_toggle();

        if (tick == 0) {
            ss_printf(4, "FM: %f \r\n", value);
        }
        tick = (tick + 1) % 50;

        ss_rtos_delay_ms(10);
    }
}

int main(void) {
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_uart_init(4, 115200));
    SS_ERROR_ASSERT(ss_fm_init(fm_pin, 1000000));

    ss_printf(4, "fse_pb_bsp example: fm \r\n");

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
