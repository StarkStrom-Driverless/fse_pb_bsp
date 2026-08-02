#include "ss.h"

#include "pins.h"

uint16_t input = EXAMPLE_INPUT_PIN;

void example_task(void* args) {

    for (;;) {
        if (ss_io_read(input)) {
            ss_led_heartbeat_toggle();
            ss_printf(4, "INPUT: 1 \r\n");
            ss_rtos_delay_ms(500);
        } else {
            ss_printf(4, "INPUT: 0 \r\n");
            ss_rtos_delay_ms(100);
        }
    }
}

int main(void) {
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_uart_init(4, 115200));
    SS_ERROR_ASSERT(ss_io_init(input, SS_GPIO_MODE_INPUT_PU));

    ss_printf(4, "fse_pb_bsp example: input \r\n");

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
