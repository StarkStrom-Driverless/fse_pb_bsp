#include "ss.h"

#include "pins.h"

uint16_t output = EXAMPLE_OUTPUT_PIN;

void example_task(void* args){
    uint8_t state = 0;

    for(;;) {
        ss_io_write(output, SS_GPIO_TOGGLE);

        state = !state;

        ss_printf(4, "OUTPUT: %d \r\n", state);

        ss_rtos_delay_ms(1000);
    }
}

int main(void) {
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_uart_init(4, 115200));
    SS_ERROR_ASSERT(ss_io_init(output, SS_GPIO_MODE_OUTPUT));

    ss_printf(4, "fse_pb_bsp example: output \r\n");

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
