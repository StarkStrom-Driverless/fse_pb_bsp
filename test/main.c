#include "ss.h"

#include "ss_canboot.h"

void ss_rtos_init_error(SS_FEEDBACK feedback);



struct SS_ETH_PAYLOAD payload;

static void task1(void *args __attribute((unused))) {
    struct SS_CAN_FRAME frame;
    ss_can_frame_set_common(&frame, 0x100, 2);
    uint8_t cnt = 0;
    uint16_t adc_val;
	for (;;) {
		ss_led_heartbeat_toggle();
        
        if (ss_adc_read(PIN('A', 0), &adc_val) == SS_FEEDBACK_OK) {
            ss_can_frame_set_signal(&frame, 0, 16, adc_val);
            ss_can_send(1, &frame);
        }

		ss_rtos_delay_ms(100);
	}
}

static void task2(void* args) {
    struct SS_CAN_FRAME frame;
    struct SS_CAN_MSG_QUEUE* queue;

    ss_can_queue_get(1, 0x123, &queue);
    for (;;) {

        if(ss_can_queue_read(queue, &frame) == SS_FEEDBACK_CAN_MSG_RECEIVED ) {
            ss_led_dbg1_toggle();
        }

        
        ss_rtos_delay_ms(100);
    }
}

static void task3(void* args) {
    struct SS_CAN_FRAME frame;
    struct SS_CAN_MSG_QUEUE* queue;

    ss_can_queue_get(2, 0x234, &queue);
    for (;;) {

        if(ss_can_queue_read(queue, &frame) == SS_FEEDBACK_CAN_MSG_RECEIVED ) {
            ss_led_dbg1_toggle();

            frame.std_id = 0x20;
            ss_can_send(1, &frame);
        }

        
        ss_rtos_delay_ms(100);
    }
}

static void task4(void* args) {
    struct SS_CAN_FRAME frame;
    struct SS_CAN_MSG_QUEUE* queue;

    ss_can_queue_get(1, 0x124, &queue);
    for (;;) {

        if(ss_can_queue_read(queue, &frame) == SS_FEEDBACK_CAN_MSG_RECEIVED ) {
            ss_led_dbg2_toggle();
        }

        
        ss_rtos_delay_ms(100);
    }
}

static void udp_recv_task(void *args __attribute((unused))) {
    /*
    struct SS_ETH_PAYLOAD* payload_ptr;
    struct SS_ETH_INTF* intf;
    for (;;) {
        ss_eth_get(5432, &intf);

        if (ss_eth_read(intf, &payload_ptr) == SS_FEEDBACK_ETH_MSG_RECEIVED) {
            ss_led_dbg2_toggle();

            //ss_eth_send(intf, )
        }

        ss_rtos_delay_ms(10);
    }
        */
}
#include <libopencm3/cm3/scb.h>


int main(void)
{
    SCB_VTOR = 0x08020200;

    SS_HANDLE_INIT(ss_clock_init(SS_CLOCK_FAST));

    SS_HANDLE_INIT(ss_leds_init());

    SS_HANDLE_INIT(ss_eth_init(SS_ETH_IP(192, 168, 10, 32), SS_ETH_IP(255, 255, 255, 0)));
    SS_HANDLE_INIT(ss_eth_socket_udp_add(5432, &payload));

    

    SS_HANDLE_INIT(ss_can_init(1, 1000000));
    SS_HANDLE_INIT(ss_can_filter_add_msg(1, 0x123));
    SS_HANDLE_INIT(ss_can_filter_add_msg(1, 0x1234));

    SS_HANDLE_INIT(ss_can_queue_handle_add(1, 0x124, task4, NULL, 2));

    SS_HANDLE_INIT(ss_can_init(2, 1000000));
    SS_HANDLE_INIT(ss_can_filter_add_msg(2, 0x234));
    SS_HANDLE_INIT(ss_can_filter_add_msg(2, 0x2345));

    SS_HANDLE_INIT(ss_rtos_task_add(task1, &ss_system, 2, "task1"));
    SS_HANDLE_INIT(ss_rtos_task_add(task2, &ss_system, 2, "task2"));
    SS_HANDLE_INIT(ss_rtos_task_add(task3, &ss_system, 2, "task3"));
    //SS_HANDLE_INIT(ss_rtos_task_add(udp_recv_task, &ss_system, 3, "udp"));


    SS_HANDLE_INIT(ss_canboot_init(0x80, 0x08080000));

    SS_HANDLE_INIT(ss_pwm_init(PIN('A', 1), 10000));

    SS_HANDLE_INIT(ss_iob_init());
    SS_HANDLE_INIT(ss_iob_add(PIN('A', 2), SS_GPIO_RAISING));

    SS_HANDLE_INIT(ss_adc_init(PIN('A', 0)));
    
    ss_rtos_start();

    while (1) {

	}

    return 0;

}

void ss_init_error(SS_FEEDBACK feedback) {
    uint16_t error = 0xFFFF & feedback;
    uint16_t top_error = (feedback >> 16);
    while(1) {
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

