#include "ss.h"

void example_task(void* args) {
    uint8_t value = 0;
    struct SS_CAN_FRAME msg;

    for (;;) {
        ss_can_frame_set_common(&msg, 0x123, 1);
        ss_can_frame_set_signal(&msg, 0, 8, value);

        ss_can_send(1, &msg);

        ss_printf(4, "TX: %d \r\n", value);

        value++;

        ss_rtos_delay_ms(500);
    }
}

int main(void) {
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_uart_init(4, 115200));
    SS_ERROR_ASSERT(ss_can_init(1, 1000000));

    ss_printf(4, "fse_pb_bsp example: can_tx \r\n");

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
