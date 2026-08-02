#define USE_PRIVATE
#include "ss.h"

uint8_t timeout_detected = 0;

void can_task(void* args) {
    struct SS_CAN_MSG_QUEUE *queue;
    struct SS_CAN_FRAME msg;

    ss_can_queue_add(1, 0x123, &queue);

    for (;;) {
        if (ss_can_queue_read(queue, &msg)) {
            ss_led_heartbeat_toggle();

            ss_can_tod_update(1, 0x123);

            ss_printf(4, "CAN_TOD: msg received \r\n");
        }

        ss_rtos_delay_ms(200);
    }
}

void state_task(void* args) {

    for (;;) {
        if (timeout_detected == 1) {
            ss_led_error_toggle();
        } else {
            ss_led_error_off();
        }

        ss_rtos_delay_ms(200);
    }
}

void tod_task(void* args) {
    uint8_t last_state = 0;

    for (;;) {
        timeout_detected = ss_can_tod_check() ? 1 : 0;

        if (timeout_detected != last_state) {
            ss_printf(4, "CAN_TOD: timeout %s \r\n", timeout_detected ? "detected" : "cleared");
            last_state = timeout_detected;
        }

        ss_rtos_delay_ms(500);
    }
}

int main(void) {
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_uart_init(4, 115200));
    SS_ERROR_ASSERT(ss_can_init(1, 1000000));
    SS_ERROR_ASSERT(ss_can_tod_add(1, 0x123, 10));

    ss_printf(4, "fse_pb_bsp example: can_tod \r\n");

    SS_ERROR_ASSERT(ss_rtos_task_add(can_task, NULL, 1, "can_task"));
    SS_ERROR_ASSERT(ss_rtos_task_add(state_task, NULL, 4, "state_task"));
    SS_ERROR_ASSERT(ss_rtos_task_add(tod_task, NULL, 1, "tod_task"));

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
