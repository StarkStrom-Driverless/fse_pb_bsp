
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

#include "ads131.h"

#define REAR 0
#define FRONT 1
#define LV_VOLTAGE_SAMPLES 50
 

struct SystickCntr systick_cntr;
struct Fifo can_receive_fifos[2];
struct SS_ADC ss_adc;
struct IOB iob;


static adc_channel_data adc_data;

 
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
    uint8_t adc_handle = ss_systick_add_handle(&systick_cntr, 10);
    uint8_t sdc_handle = ss_systick_add_handle(&systick_cntr, 100);
    uint8_t ads_handle = ss_systick_add_handle(&systick_cntr, 1000);
    uint8_t fade_handle = ss_systick_add_handle(&systick_cntr, 1);

    /**************************************************************************
     *                      CAN INTERFACE MODE (REAR / FRONT)                 *     
     *************************************************************************/
    uint16_t mode_pin = ss_io_init(PIN('B', 1), SS_GPIO_MODE_INPUT);
    uint8_t mode = ss_io_read(mode_pin)?1:0;


    /**************************************************************************
     *                                      CAN                               *     
     *************************************************************************/
    /*******************************************************
     *                  CAN1                               *     
     ******************************************************/
    /**
     * CAN RX FIFO INIT
     */
    init_fifo(&can_receive_fifos[0]);

    /**
     * CAN INIT
     */
    ss_can_init(1, 1000000);

    /*******************************************************
     *                  CAN1                               *     
     ******************************************************/
    /**
     * CAN RX FIFO INIT
     */
    init_fifo(&can_receive_fifos[1]);

    /**
     * CAN INIT
     */
    ss_can_init(2, 1000000);

    /**
     * CAN RX FRAME MASK INIT
     */
    uint32_t msg_ids[] = {0x190};
    ss_can_add_messages(msg_ids, 1);


    


    /**************************************************************************
     *                                      BOARD LEDS                        *     
     *************************************************************************/
    ss_leds_init();

    /**************************************************************************
     *                                      ADC INIT                          *     
     *************************************************************************/
    uint16_t lv_voltage_pin = ss_adc_init(PIN('A', 0), &ss_adc);
    uint64_t lv_voltage = 0;
    uint16_t lv_voltages[LV_VOLTAGE_SAMPLES];
    uint8_t lv_voltage_pos = 0;
    ss_adc_start(&ss_adc);

    /**************************************************************************
     *                                      IO OBSERVATION                    *     
     *************************************************************************/
    ss_iob_init(&iob);
    uint16_t sdc_mon_1_pin = ss_iob_add(PIN('A', 11), &iob, SS_GPIO_RAISING);
    uint16_t sdc_mon_2_pin = ss_iob_add(PIN('B', 12), &iob, SS_GPIO_RAISING);
    uint16_t sdc_mon_3_pin = ss_iob_add(PIN('B', 13), &iob, SS_GPIO_RAISING);
    uint16_t sdc_mon_4_pin = ss_iob_add(PIN('B', 14), &iob, SS_GPIO_RAISING);
    uint16_t sdc_mon_5_pin = ss_iob_add(PIN('B', 15), &iob, SS_GPIO_RAISING);

    /**************************************************************************
     *                                BREAKLIGHT                              *     
     *************************************************************************/
    uint16_t breaklight_pin = ss_pwm_init(PIN('A', 15), 10000, 16000000);
    ss_pwm_write(breaklight_pin, 0);
    uint8_t breaklight_value = 0;
    uint8_t breaklight_status = 0;

    /**************************************************************************
     *                                LUEFTER                              *     
     *************************************************************************/
    uint16_t pwm1 = ss_pwm_init(PIN('A', 1), 10000, 16000000);
    uint16_t pwm2 = ss_pwm_init(PIN('A', 2), 10000, 16000000);
    uint16_t pwm3 = ss_pwm_init(PIN('B', 0), 10000, 16000000);
    uint16_t pwm4 = ss_pwm_init(PIN('C', 6), 10000, 16000000);
    ss_pwm_write(pwm1, 100);
    ss_pwm_write(pwm2, 100);
    ss_pwm_write(pwm3, 100);
    ss_pwm_write(pwm4, 100);
    uint8_t pwm_val = 100;

    /**************************************************************************
     *                                EXTERN ADC                              *     
     *************************************************************************/
    /**
     * SPI
     */
    ss_spi_init(1, 1000000);

    /**
     * ADC131
     */
    init_ads131_wrapper(); 
    adcStartup();




    while (1) {
        /**
         * PWM
         */
        ss_pwm_write(pwm1, pwm_val);
        ss_pwm_write(pwm2, pwm_val);
        ss_pwm_write(pwm3, pwm_val);
        ss_pwm_write(pwm4, pwm_val);

        /**
         * BREAKLIGHT
         */

        if (ss_systick_expired(&systick_cntr, fade_handle)) {
            if (breaklight_status == 1) {
                breaklight_value = (breaklight_value < 100) ? breaklight_value + 10 : breaklight_value;
                
            } else {
                breaklight_value = (breaklight_value > 0) ? breaklight_value - 10 : breaklight_value;

            }

            ss_pwm_write(breaklight_pin, breaklight_value / 5);
        }

        
        
        /**
         * LV-Voltage HANDLE
         */
        if(ss_systick_expired(&systick_cntr, adc_handle)) {
            lv_voltages[lv_voltage_pos] = ss_adc_read(lv_voltage_pin, &ss_adc);

            lv_voltage_pos = (lv_voltage_pos < LV_VOLTAGE_SAMPLES) ? lv_voltage_pos + 1 : 0;

            lv_voltage = 0;

            for (uint8_t i = 0; i < LV_VOLTAGE_SAMPLES; i++) {
                lv_voltage += lv_voltages[i];
            }

            lv_voltage /= LV_VOLTAGE_SAMPLES;

            lv_voltage = 0.08861 * lv_voltage + 2.0065;
            
        }
        

        /**
         * heartbeat
         */
        if (ss_systick_expired(&systick_cntr, heartbeat_handle)) {
            ss_led_heartbeat_toggle(); 
        }

        /**
         * IO Oberservation
         */
        
        if (ss_systick_expired(&systick_cntr, sdc_handle)) {

            struct can_tx_msg can_msg;
            ss_can_reset_frame(&can_msg);
            if (mode == FRONT) {
                ss_can_set_common_to_frame(&can_msg, 0x187, 1);
            } else {
                ss_can_set_common_to_frame(&can_msg, 0x186, 3);
            }

            uint8_t sdc_1 = ss_iob_get(sdc_mon_1_pin, &iob);
            uint8_t sdc_2 = ss_iob_get(sdc_mon_2_pin, &iob);
            uint8_t sdc_3 = ss_iob_get(sdc_mon_3_pin, &iob);
            uint8_t sdc_4 = ss_iob_get(sdc_mon_4_pin, &iob);
            uint8_t sdc_5 = ss_iob_get(sdc_mon_5_pin, &iob);
            
            
            ss_can_set_signal_to_frame(&can_msg, 0, 1, sdc_1);
            ss_can_set_signal_to_frame(&can_msg, 1, 1, sdc_2);
            ss_can_set_signal_to_frame(&can_msg, 2, 1, sdc_3);
            ss_can_set_signal_to_frame(&can_msg, 3, 1, sdc_4);
            ss_can_set_signal_to_frame(&can_msg, 4, 1, sdc_5);

            if(mode == REAR) {
                ss_can_set_signal_to_frame(&can_msg, 7, 1, breaklight_status);
                ss_can_set_signal_to_frame(&can_msg, 14, 10, lv_voltage);
            } 
            
            
            ss_can_send(1, &can_msg);
        }

        /**
         * ADS131
         */
        if(ss_systick_expired(&systick_cntr, ads_handle)) {
            if (data_received()) {
                readData(&adc_data);

                adc_data.channel0 += (adc_data.channel0 > 8388608) ? -8388608 : 8388608;
                adc_data.channel1 += (adc_data.channel1 > 8388608) ? -8388608 : 8388608;
                adc_data.channel2 += (adc_data.channel2 > 8388608) ? -8388608 : 8388608;
                adc_data.channel3 += (adc_data.channel3 > 8388608) ? -8388608 : 8388608;


                struct can_tx_msg can_msg;
                ss_can_reset_frame(&can_msg);
                ss_can_set_common_to_frame(&can_msg, 0x100, 8);

                uint32ToUint8Array_LE(adc_data.channel0, &can_msg.data[0]);
                uint32ToUint8Array_LE(adc_data.channel1, &can_msg.data[4]);

                ss_can_send(2, &can_msg);

 
                ss_can_reset_frame(&can_msg);
                ss_can_set_common_to_frame(&can_msg, 0x101, 8);

                uint32ToUint8Array_LE(adc_data.channel2, &can_msg.data[0]);
                uint32ToUint8Array_LE(adc_data.channel3, &can_msg.data[4]);

                ss_can_send(2, &can_msg);

            }
        }
        


        /**
         * Handle Input CAN-Messages
         */
        if (!is_fifo_empty(&can_receive_fifos[0])) {

            struct can_rx_msg can_frame;
            int8_t ret = fifo_remove_can_frame(&can_receive_fifos[0], &can_frame);

            switch(can_frame.std_id) {
                case 0x190: 
                    if (mode == REAR) {
                        breaklight_status = ss_can_get_signal_from_frame(&can_frame, 0, 1);
                    }
                    pwm_val = ss_can_get_signal_from_frame(&can_frame, 33, 7);
                    if (pwm_val <= 100) {
                        pwm_val = 100 - pwm_val;
                    }

                    break;

                default: 
                    ss_led_dbg1_toggle();
                    break;
            }

        }
    
    }

    return 0;
}