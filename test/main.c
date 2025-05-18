#include "ss.h"


#define REAR 0
#define FRONT 1
#define LV_VOLTAGE_SAMPLES 50
 

struct SystickCntr systick_cntr;
struct Fifo can_receive_fifos[2];
struct SS_ADC ss_adc;
struct IOB iob;



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
    uint16_t pin = ss_io_init(PIN('A', 15), SS_GPIO_MODE_OUTPUT);
    ss_io_write(pin, SS_GPIO_OFF);


    ss_leds_init();


    struct System ss_system;


    ss_rtos_task_add(task1, &ss_system, 2, "task1");
    ss_rtos_task_add(task2, &ss_system, 2, "task2");
    ss_rtos_start();


    while (1) {

	}

    return 0;

}