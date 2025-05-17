
#include "ss_gpio.h"
#include "ss_pwm.h"
#include "ss_systick.h"
#include "ss_can.h"
#include "ss_spi.h"
#include "ss_leds.h"
#include "ss_adc.h"
#include "ss_delay.h"
#include "ss_iob.h"
#include "ss_spi.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#ifdef SS_USE_RTOS

#include "FreeRTOS.h"
#include "task.h"

extern void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName );

void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName) {
	for(;;);	// Loop forever here..
}

static void task1(void *args __attribute((unused))) {

	for (;;) {
		ss_led_heartbeat_toggle(); 
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
#endif


#define REAR 0
#define FRONT 1
#define LV_VOLTAGE_SAMPLES 50
 

struct SystickCntr systick_cntr;
struct Fifo can_receive_fifos[2];
struct SS_ADC ss_adc;
struct IOB iob;










 
int main(void)
{
    /**************************************************************************
     *                                      SYSTICK                           *     
     *************************************************************************/


    uint16_t pin = ss_io_init(PIN('A', 15), SS_GPIO_MODE_OUTPUT);
    ss_io_write(pin, SS_GPIO_OFF);



    /**************************************************************************
     *                                      BOARD LEDS                        *     
     *************************************************************************/
    ss_leds_init();






    #ifdef SS_USE_RTOS
	xTaskCreate(task1,"LED",100,NULL,configMAX_PRIORITIES-1,NULL);
	vTaskStartScheduler();
    #endif

    while (1) {

        
        
        //ss_led_heartbeat_toggle();
        

        /**
         * heartbeat
         */

        /*
        if (ss_systick_expired(&systick_cntr, heartbeat_handle)) {
            ss_led_heartbeat_toggle(); 
        }
        */


	}

    return 0;

}