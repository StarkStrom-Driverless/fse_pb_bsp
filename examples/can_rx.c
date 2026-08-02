#define USE_PRIVATE
#include "ss.h"

void example_task(void* args) {

    struct SS_CAN_MSG_QUEUE *queue;
    struct SS_CAN_FRAME msg;

    ss_can_queue_add(1, 0x22, &queue);

    for (;;) {
        if (ss_can_queue_read(queue, &msg)) {
            ss_printf(4, "ID: %x \r\n", msg.std_id);
            for (int i = 0; i < msg.dlc; i++) {
                ss_printf(4, "DATA[%d] = %x \r\n", i, msg.data[i]);
            } 

            ss_led_heartbeat_toggle();
        }

        ss_rtos_delay_ms(200);
    }
}

void example_task_multi(void* args) {
    struct SS_CAN_MSG_QUEUE *queue1;
    struct SS_CAN_MSG_QUEUE *queue2;
    struct SS_CAN_FRAME msg1;
    struct SS_CAN_FRAME msg2;

    ss_can_queue_add(1, 0x23, &queue1);
    ss_can_queue_add(1, 0x24, &queue2);

    for (;;) {
        if (ss_can_queue_read(queue1, &msg1)) {
            ss_printf(4, "ID 0x23: %x \r\n", msg1.std_id);
            ss_led_dbg1_toggle();
        }

        if (ss_can_queue_read(queue2, &msg2)) {
            ss_printf(4, "ID 0x24: %x \r\n", msg2.std_id);
            ss_led_dbg2_toggle();
        }

        ss_rtos_delay_ms(100);
    }
}

int main(void) {
    SS_ERROR_ASSERT(ss_init());
    SS_ERROR_ASSERT(ss_uart_init(4, 115200));

    ss_printf(4, "fse_pb_bsp example: can_rx \r\n");

    SS_ERROR_ASSERT(ss_can_init(1, 1000000));

    SS_ERROR_ASSERT(ss_rtos_task_add(example_task, NULL, 1, "example_task"));
    SS_ERROR_ASSERT(ss_rtos_task_add(example_task_multi, NULL, 1, "example_task_multi"));

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
