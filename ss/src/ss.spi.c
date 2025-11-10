/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include <inttypes.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>

#include "ss_gpio.h"
#include "ss_spi.h"
#include "ss_clock.h"

#define SS_FEEDBACK_BASE SS_FEEDBACK_SPI_INIT_ERROR


SS_FEEDBACK ss_enable_spi_gpios(uint8_t spi_interface_id) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    uint16_t miso = 0;
    uint16_t mosi = 0;
    uint16_t sck = 0;
    
    switch (spi_interface_id) {
        case 1:
            miso = PIN('A', 6);
            mosi = PIN('A', 7);
            sck = PIN('A', 5);
            break;

        case 2: 
            miso = PIN('B', 14);
            mosi = PIN('B', 15);
            sck = PIN('B', 13);
            break;

        case 3: 
            miso = PIN('C', 11);
            mosi = PIN('C', 12);
            sck = PIN('C', 10);
            break;

        default:
            rc = SS_FEEDBACK_SPI_GPIO_INIT_ERROR;

            break;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_io_init(miso, GPIO_MODE_AF);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_io_init(mosi, GPIO_MODE_AF);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_io_init(sck, GPIO_MODE_AF);
    SS_HANDLE_ERROR_WITH_EXIT(rc);


    uint8_t af = 0;
    switch(spi_interface_id) {
        case 1:
        case 2:
            af = GPIO_AF5;
            break;
        
        case 3:
            af = GPIO_AF6;
            break;

        default:
            rc = SS_FEEDBACK_SPI_GPIO_INIT_ERROR;
            break;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

       
    gpio_set_af(GPIO(PINBANK(miso)), af, BIT(PINNO(miso)));
    gpio_set_af(GPIO(PINBANK(mosi)), af, BIT(PINNO(mosi)));
    gpio_set_af(GPIO(PINBANK(sck)), af, BIT(PINNO(sck)));


    return rc;
}

SS_FEEDBACK ss_enable_spi_rcc(uint8_t can_interface_id) {
    SS_FEEDBACK status = SS_FEEDBACK_OK;

    switch(can_interface_id) {
        case 1:
            rcc_periph_clock_enable(RCC_SPI1);
            break;

        case 2:
            rcc_periph_clock_enable(RCC_SPI2);
            break;

        case 3:
            rcc_periph_clock_enable(RCC_SPI3);
            break;

        default:
            status = SS_FEEDBACK_SPI_RCC_INIT_ERROR;
            break;
    }

    return status;
}

uint32_t ss_get_spi_port_from_id(uint8_t spi_interface_id) {
    uint32_t spi_port = 0;
    switch(spi_interface_id) {
        case 1:
            spi_port = SPI1;
            break;

        case 2:
            spi_port = SPI2;
            break;

        case 3:
            spi_port = SPI3;
            break;

        default:
            break;
    }

    return spi_port;
}



SS_FEEDBACK ss_spi_init(uint8_t spi_interface_id, uint32_t baudrate)
{
    SS_FEEDBACK rc;

    uint32_t spi_port = ss_get_spi_port_from_id(spi_interface_id);

    rc = ss_enable_spi_rcc(spi_interface_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_enable_spi_gpios(spi_interface_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);


    uint32_t spi_prescaler = 0;
    rc = ss_clock_spi(&spi_prescaler, baudrate, spi_interface_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);


    spi_disable(spi_port);

    spi_set_master_mode(spi_port);
    spi_set_dff_8bit(spi_port);
    spi_send_msb_first(spi_port);
    spi_set_full_duplex_mode(spi_port);

    spi_set_clock_polarity_0(spi_port);   // CPOL=0
    spi_set_clock_phase_0(spi_port);      // CPHA=0

    spi_disable_software_slave_management(spi_port);
    spi_enable_software_slave_management(spi_port);  // SSM=1
    spi_set_nss_high(spi_port);                       // SSI=1

    spi_set_baudrate_prescaler(spi_port, spi_prescaler);

    spi_enable(spi_port);
    return SS_FEEDBACK_OK;
}

int8_t ss_spi_rxtx(uint8_t spi_interface_id, uint8_t* rx, uint8_t* tx, uint16_t num) {
    uint32_t spi_port = ss_get_spi_port_from_id(spi_interface_id);

    if (spi_port == 0) return -1;

    for (uint16_t i = 0; i < num; i++) {
        spi_send(spi_port, (uint8_t)tx[i]);
        rx[i] = spi_read(spi_port);
    }

    return 0;
}