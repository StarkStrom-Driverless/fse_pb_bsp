#include "ss.h"

void example_task(void* args) {
    uint8_t tx[4] = {0x1, 0x2, 0x3, 0x4};
    uint8_t rx[4] = {0};

    for (;;) {
        ss_spi_rxtx(1, rx, tx, 4);

        for (int i = 0; i < 4; i++) {
            ss_printf(4, "RX[%d] = %x \r\n", i, rx[i]);
        }

        ss_rtos_delay_ms(500);
    }
}

int main(void) {
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_uart_init(4, 115200));
    SS_ERROR_ASSERT(ss_spi_init(1, 1000000, 0));

    ss_printf(4, "fse_pb_bsp example: spi \r\n");

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
