#include <inttypes.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>

#include "ss_gpio.h"
#include "ss_spi.h"
#include "ss_clock.h"


int8_t ss_enable_spi_gpios(uint8_t spi_interface_id) {
    int8_t status = 0;

    uint16_t miso = 0;
    uint16_t mosi = 0;
    uint16_t sck = 0;
    
    switch (spi_interface_id) {
        case 1:
            miso = ss_io_init(PIN('A', 6), GPIO_MODE_AF);
            mosi = ss_io_init(PIN('A', 7), GPIO_MODE_AF);
            sck = ss_io_init(PIN('A', 5), GPIO_MODE_AF);
            break;

        case 2: 
            miso = ss_io_init(PIN('B', 14), GPIO_MODE_AF);
            mosi = ss_io_init(PIN('B', 15), GPIO_MODE_AF);
            sck = ss_io_init(PIN('B', 13), GPIO_MODE_AF);
            break;

        case 3: 
            miso = ss_io_init(PIN('C', 11), GPIO_MODE_AF);
            mosi = ss_io_init(PIN('C', 12), GPIO_MODE_AF);
            sck = ss_io_init(PIN('C', 10), GPIO_MODE_AF);
            break;

        default:
            status = -1;
            break;
    }

    if (status == 0) {
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
                status = -1;
                break;
        }

        if (status == -1) return status;

        gpio_set_af(GPIO(PINBANK(miso)), af, BIT(PINNO(miso)));
        gpio_set_af(GPIO(PINBANK(mosi)), af, BIT(PINNO(mosi)));
        gpio_set_af(GPIO(PINBANK(sck)), af, BIT(PINNO(sck)));
    }

    return status;
}

int8_t ss_enable_spi_rcc(uint8_t can_interface_id) {
    int8_t status = 0;

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
            status = -1;
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
    uint32_t spi_port = ss_get_spi_port_from_id(spi_interface_id);


    if (ss_enable_spi_rcc(spi_interface_id) == -1)
        return SET_TOPLEVEL_ERROR(SS_FEEDBACK_SPI_INIT_ERROR, SS_FEEDBACK_RCC_INIT_ERROR);
    if (ss_enable_spi_gpios(spi_interface_id) == -1)
        return SET_TOPLEVEL_ERROR(SS_FEEDBACK_SPI_INIT_ERROR, SS_FEEDBACK_SPI_GPIO_INIT_ERROR);


    uint32_t spi_prescaler = 0;
    if (ss_clock_spi(&spi_prescaler, baudrate, spi_interface_id) != SS_FEEDBACK_OK)
        return SET_TOPLEVEL_ERROR(SS_FEEDBACK_SPI_INIT_ERROR, SS_FEEDBACK_CLOCK_SPI_INIT_ERROR);


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