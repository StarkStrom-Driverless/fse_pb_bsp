#include <inttypes.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>

#include "ss_gpio.h"
#include "ss_spi.h"


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

uint32_t ss_get_spi_prescaler(uint32_t baudrate, uint32_t clk) {
    uint32_t prescaler = 0;
    
    uint32_t br = clk / baudrate;
    br = (br < 2 || br > 256) ? 2 : br;

    switch(br) {
        case 2: prescaler = SPI_CR1_BR_FPCLK_DIV_2; break;
        case 4: prescaler = SPI_CR1_BR_FPCLK_DIV_4; break;
        case 8: prescaler = SPI_CR1_BR_FPCLK_DIV_8; break;
        case 16: prescaler = SPI_CR1_BR_FPCLK_DIV_16; break;
        case 32: prescaler = SPI_CR1_BR_FPCLK_DIV_32; break;
        case 64: prescaler = SPI_CR1_BR_FPCLK_DIV_64; break;
        case 128: prescaler = SPI_CR1_BR_FPCLK_DIV_128; break;
        case 256: prescaler = SPI_CR1_BR_FPCLK_DIV_256; break;
        default: prescaler = SPI_CR1_BR_FPCLK_DIV_256; break;
    }

    return prescaler;
}

int8_t ss_spi_init(uint8_t spi_interface_id, uint32_t baudrate) {
    uint8_t status = 0;

    uint32_t spi_port = ss_get_spi_port_from_id(spi_interface_id);

    uint32_t spi_prescaler = ss_get_spi_prescaler(baudrate, 16000000);

    if (ss_enable_spi_rcc(spi_interface_id) == -1) status = -1;

    if (ss_enable_spi_gpios(spi_interface_id) == -1) status = -1;

    int ret = spi_init_master(  spi_port,
                                spi_prescaler,
                                SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                                SPI_CR1_CPHA_CLK_TRANSITION_1,
                                SPI_CR1_DFF_8BIT,
                                SPI_CR1_MSBFIRST);

    if (ret == 1) status = -1;

    spi_set_full_duplex_mode(spi_port);

    spi_set_clock_phase_0(spi_port);

    spi_set_clock_polarity_0(spi_port);

    spi_enable_software_slave_management(spi_port);

    spi_set_nss_high(spi_port);

    spi_enable(spi_port);

    return status;
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