# fse_pb_bsp

- [fse\_pb\_bsp](#fse_pb_bsp)
  - [Introduction](#introduction)
  - [Installation](#installation)
  - [Flow](#flow)
  - [Programming](#programming)
    - [Output](#output)
    - [Input](#input)
    - [PWM](#pwm)
    - [Systick](#systick)
    - [CAN](#can)
    - [ADC](#adc)
  - [PIN capabilities](#pin-capabilities)
  - [PORTA](#porta)
  - [PORTB](#portb)
  - [PORTC](#portc)


## Introduction
- Formula Student Electric 
- Processor Board
- Board Support Package

This is a library for the Starkstrom Augsburg processorboard.
The processorboard is a circuit which includes a STM32F405, a 2-Channel CAN-Interface which is connected to the STM and GPIO's.
This circuit is connected via an M.2 slot to our controll-units placed in the car.

## Installation
The installation description is for a Linux based system.
This wont work for a Windof based system. You can try to use WSL2.
Maybe you run into some trouble when it comes to flashing the software.
So for that, the derived repositories which contains the actuall software for the controll-units should have a release which can be directly loaded into the controller via the st-link. 

```
sudo apt install binutils-arm-none-eabi gcc-arm-none-eabi
sudo apt install gdb-multiarch
cd /usr/bin
sudo ln -s gdb-multiarch arm-none-eabi-gdb

# Download this arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz on
# https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

cd /opt
sudo tar Jxvf ~/arm-gnu-toolchain-12.3.rel1-x86_64-arm-none-eabi.tar.xz

export PATH=$PATH:/opt/arm-gnu-toolchain-12.3.rel1-x86_64-arm-none-eabi/bin

sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt update
sudo apt install python3.8

sudo apt install openocd

sudo cp /lib/udev/rules.d/60-openocd.rules /etc/udev/rules.d/
```

## Flow
```
# if the gdb-part is commented out in the makefile
# compiling
make 

# flashing + compiling
make flash

# debugging, if the gdb-part is included in the makefile
# for this the gdb-server needs to be called in a seperate terminal
make gdb

# Start gdb-server
# goto the fse_pb_bsp directory and type
openocd -f tools/openocd.cfg
```

## Programming

### Output
```c 
#include "ss_gpio.h"
#include "ss_delay.h"

int main(void) {

    uint16_t pa2 = ss_io_init(PIN('A', 2),  SS_GPIO_MODE_OUTPUT);

    for (;;) {
        ss_gpio_write(PA2,  GPIO_TOGGLE); // SS_GPIO_ON -  SS_GPIO_OFF
        delay(100); //delay in processor cycles
    }

    return 0;
}
```

### Input
```c 
#include "ss_gpio.h"

int main(void) {

    uint16_t pa2 = ss_io_init(PIN('A', 2),  SS_GPIO_MODE_OUTPUT);
    uint16_t pa3 = ss_io_init(PIN('A', 3), SS_GPIO_MODE_INPUT);

    for (;;) {
        if (ss_io_read(pa3)) {
            ss_gpio_write(PA2,  SS_GPIO_ON);
        } else {
            ss_gpio_write(PA2,  SS_GPIO_ON);
        }
    }

    return 0;
}
```

### PWM
```c 
#include "ss_pwm.h"

int main(void) {

    // 50kHz PWM
    // 16MHz Sys-Tackt
    uint16_t pa2 = ss_pwm_init(PIN('A', 2), 50000, 16000000);

    for (;;) {
        // PWM-Values between 0 and 100
        ss_pwm_write(pa2, 75);
    }

    return 0;
}
```


### Systick
The following thing is nasty. I know it and if a have enough time i will change it.

To use the systick-stuff you have to change stuff insight the library. So make sure you make a seperate branch in fse_pb_bsp.



- main.c
```c 
#include "ss_gpio.h"
#include "ss_systick.h"

// Define the systick-handling struct
struct SystickCntr systick_cntr;

int main(void) {

    // Systickhandler is called after 16MHz / 1600 Processor-cycles 
    ss_init_systick(1600);

    /**
     *  Init the Systick handling
     *  - With add_handle you can create a new handler with a given execution period 
     *  - add_handle returns a id of the handler which can be used in the main-loop
     */
    ss_init_systick_cntr(&systick_cntr);
    uint8_t heartbeat_handle = ss_systick_add_handle(&systick_cntr, 2000);


    for (;;) {

        /**
         *  systick_expired is the function which returns true if the parallel handler is expired after the defined handle-period.
         * This is a very simple non blocking scheduling of "tasks"
         * - pass the handler-struct systick_cntr as a pointer 
         * - pass the handler-id from add_handler
         */
        if (ss_systick_expired(&systick_cntr, heartbeat_handle)) {
            ss_led_heartbeat_toggle(); 
        }
    }

    return 0;
}
```

The if statement is executed after 200 systickhandler interrupts

### CAN
```c 
#include "ss_gpio.h"
#include "ss_systick.h"
#include "ss_can.h"


Systick_Handle handle1 = {.timer = 0, .period=200, .tick = 0};

// For receiving define this input-fifo
struct Fifo can_receive_fifos[2];

int main(void) {

    // Systickhandler is called after 16MHz / 1600 Processor-cycles 
    ss_init_systick(1600);

    /*
    *   init the fifo with index 0 for CAN1 and 1 for CAN2
    *   init the can with the CAN-Index 1-CAN1 and 2-CAN2 and the baudrate
    *   define input-filter messages
    *   pass the input-filter messages with the lenght of the array
    */
    init_fifo(&can_receive_fifos[0]);
    ss_can_init(1, 1000000);
    uint32_t msg_ids[] = {0x125, 0x111, 0x33, 0x01};
    ss_can_add_messages(msg_ids, 4);

    uint16_t pa2 = ss_io_init(PIN('A', 2),  SS_GPIO_MODE_OUTPUT);

    for (;;) {

        /**
         *   Sending CAN-Messages
         *   create a can_tx_message struct and write the can-frame to it
         *   call ss_can_send with the can-index and a pointer to the frame
         */
        if (ss_handle_timer(&handle2)) {
            struct can_tx_msg can_frame;
            can_frame.std_id = 0x111;
            can_frame.ide = 0;
            can_frame.rtr = 0;
            can_frame.dlc = 3;
            can_frame.data[0] = (0x00FF & adc_val);
            can_frame.data[1] = (adc_val >> 8);
            can_frame.data[2] = ss_io_read(fault_pin);

            ss_can_send(1, &can_frame);
        }

        /**
         *  Receive CAN-Frames from FIFO
         *  if fifo is not empty.
         *  define a can_rx_frame struct and copy the can-frame from the fifo with fifo_remove_can_frame.
         *  use switch-case to handle the reaction based on the can-id
         */
        if (!is_fifo_empty(&can_receive_fifos[0])) {
            
            struct can_rx_msg can_frame;
            fifo_remove_can_frame(&can_receive_fifos[0], &can_frame);

            switch(can_frame.std_id) {
                case 0x125:
                    
                    ssvesc_handle(	&can_frame,
                                    pwm_pin,
                                    dir_pin,
                                    en_pin,
                                    break_pin, 
                                    fault);
                    
                    ss_led_dbg1_toggle();
                    break;

                case 0xc1:
                    break;

                default: break;
            }
            
        }
    }

    return 0;
}
```

### ADC
The ADC API provides an interface to use the onboard ADC's of the stm

It can be used like this.
- This example demonstrades a periodic read of an adc-value
- The adc-value which is reat is updated in the background
- So this is non blockin also

```C
#include "ss_adc.h"
#include "ss_systick.h"

struct SystickCntr systick_cntr;

// define the adc-handling struct
struct SS_ADC ss_adc;


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
    uint8_t adc_handle = ss_systick_add_handle(&systick_cntr, 1000);




    /**************************************************************************
     *                                      ADC INIT                          *     
     *************************************************************************/
    /**
     *  - init every adc-channel sperate with the pin-id and the adc-handle struct as a pointer
     *  - adc_init also returns the pin-id for the adc-channel
     *  - After all adc-channels are initialised call adc_start
     **/
    uint16_t adc_pin = ss_adc_init(PIN('A', 0), &ss_adc);
    ss_adc_start(&ss_adc);




    while (1) {
        
        /**
         * Read adc-value for a channel after adc_handle time is expired
         * adc_read needs the pin-id which and a pointer of the adc handle
         */
        if(ss_systick_expired(&systick_cntr, adc_handle)) {
            uint16_t value = ss_adc_read(adc_pin, &ss_adc);

        }
        
    }

    return 0;
}
```

## PIN capabilities

## PORTA
|PIN| GPIO|PWM|CAN|SPI1|SPI2|SPI3|UART|ADC
|-|-|-|-|-|-|-|-|-|
|PA0|True|True||||||True
|PA1|True|True||||||True
|PA2|True|True|||||UART2_Tx|True
|PA3|True|True|||||UART2_Rx|True
|PA4|True|||SPI1_NSS||SPI3_NSS||True
|PA5|True|True||SPI1_SCK||||True
|PA6|True|True||SPI1_MISO||||True
|PA7|True|True||SPI1_MOSI||||True
|PA8|True|True|||||||
|PA9|True|True|||||UART1_Rx||
|PA10|True|True|||||UART1_Tx||
|PA11|True|True|||||||
|PA12|True||||||||
|PA13|||||||||
|PA14|||||||||
|PA15|True|True||SPI1_NSS||SPI3_NSS|||

## PORTB
|PIN| GPIO|PWM|CAN|SPI1|SPI2|SPI3|UART|ADC
|-|-|-|-|-|-|-|-|-|
|PB0|True|True||||||True
|PB1|True|True||||||True
|PB2|||||||||
|PB3||||SPI1_SCK||SPI3_SCK|||
|PB4|||STB2|SPI1_MISO||SPI3_MISO|||
|PB5|||RX2|SPI1_MOSI||SPI3_MOSI|||
|PB6|||TX2||||||
|PB7|||STB1||||||
|PB8|||RX1||||||
|PB9|||TX1||SPI2_NSS||||
|PB10|True|True|||SPI2_SCK||UART3_Rx - UART4_Rx||
|PB11|True|True|||||UART3_Tx||
|PB12|True||||SPI2_NSS||||
|PB13|True||||SPI2_SCK||||
|PB14|True|True|||SPI2_MISO||||
|PB15|True|True|||SPI2_MOSI||||

## PORTC
|PIN| GPIO|PWM|CAN|SPI1|SPI2|SPI3|UART|ADC
|-|-|-|-|-|-|-|-|-|
|PC0|||||||||
|PC1|||||||||
|PC2|True|||||||True
|PC3|True|||||||True
|PC4|||||||||
|PC5|||||||||
|PC6|True|True|||||UART6_Rx||
|PC7|True|True|||||UART6_Rx||
|PC8|True|True|||||||
|PC9|True|True|||||||
|PC10|True|||||SPI3_SCK|||
|PC11|True|||||SPI3_MISO|UART4_Tx||
|PC12|True|||||SPI3_MOSI|||
|PC13|True||||||||
|PC14|True||||||||
|PC15|True||||||||

