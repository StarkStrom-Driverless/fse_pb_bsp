#include "ss.h"

#include "pins.h"

uint16_t input = EXAMPLE_IOB_PIN;

void example_task(void* args) {

    for (;;) {
        if (ss_iob_get(input) == 0) {
            ss_led_heartbeat_toggle();
            ss_printf(4, "IOB: triggered \r\n");
        }

        ss_rtos_delay_ms(500);
    }
}

int main(void) {
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_uart_init(4, 115200));
    SS_ERROR_ASSERT(ss_iob_add(input, SS_GPIO_FALLING));

    ss_printf(4, "fse_pb_bsp example: iob \r\n");

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
