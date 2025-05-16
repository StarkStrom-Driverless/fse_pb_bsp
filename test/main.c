
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
    /**
     * SYSTICK INIT
     */
    ss_init_systick(160);

    /**
     * SYSTICK HANDLE
     */
    ss_init_systick_cntr(&systick_cntr);
    uint8_t heartbeat_handle = ss_systick_add_handle(&systick_cntr, 2000);



    /**************************************************************************
     *                                      BOARD LEDS                        *     
     *************************************************************************/
    ss_leds_init();








    while (1) {

        
        

        

        /**
         * heartbeat
         */
        if (ss_systick_expired(&systick_cntr, heartbeat_handle)) {
            ss_led_heartbeat_toggle(); 
        }


	}

    return 0;

}