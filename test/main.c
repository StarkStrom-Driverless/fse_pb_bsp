#include "ss.h"

uint16_t led = PIN('C', 1);

static void blinky_task(void *args) {
    uint8_t value = 0;

    for (;;) {
        ss_io_write(led, value);

        value = (value == SS_GPIO_ON) ? SS_GPIO_OFF : SS_GPIO_ON;

        ss_rtos_delay_ms(500);
    }
}

int main(void)
{
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_io_init(led, SS_GPIO_MODE_OUTPUT));

    SS_ERROR_ASSERT(ss_rtos_task_add(blinky_task, NULL, 1, "blinky_task"));

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

