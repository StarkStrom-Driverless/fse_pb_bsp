#include "ss.h"






struct System {
    int a;
};

static void task1(void *args __attribute((unused))) {
	for (;;) {
		ss_led_heartbeat_toggle(); 
		ss_rtos_delay_ms(200);
	}
}

static void task2(void* args) {
    for (;;) {
        ss_led_dbg1_toggle();
        ss_rtos_delay_s(2);
    }
}




 
int main(void)
{



    ss_leds_init();


    struct System ss_system;


    ss_rtos_task_add(task1, &ss_system, 2, "task1");
    ss_rtos_task_add(task2, &ss_system, 2, "task2");
    ss_rtos_start();


    while (1) {

	}

    return 0;

}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName) {
    while(1) {
        ss_led_error_on();
    }
}