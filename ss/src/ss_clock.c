/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#define USE_PRIVATE
#include "ss_config.h"

#if COMPILE_SS_CLOCK
#include "ss_clock.h"
#include "ss_gpio.h"
#include "ss_makros.h"
#include "ss_error.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>

struct SS_CLOCK ss_clock;

bool ss_clock_init(uint8_t config) {

    ss_clock.ahb = 16;
    ss_clock.apb1 = 16;
    ss_clock.apb2 = 16;

    switch (config)
    {
        case SS_CLOCK_DEFAULT:
        break;

        case SS_CLOCK_FAST:
            {
                ss_clock.ahb = 168;
                ss_clock.apb1 = 42;
                ss_clock.apb2 = 84;
                rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
            }
        break;

        default:
            SS_ERROR("unknown clock config");
    }

    return true;
}

bool ss_clock_can(struct SS_CLOCK_CAN* config, uint32_t baudrate) {
    switch (baudrate)
    {
    case 1000000:
        switch (ss_clock.apb1)
        {
            case 16:
                {
                    config->prescaler = 1;
                    config->tseg1 = CAN_BTR_TS1_12TQ;
                    config->tseg2 = CAN_BTR_TS2_3TQ;
                    config->sjw = CAN_BTR_SJW_1TQ;
                }
            break;

            case 42:
                {
                    config->prescaler = 3;
                    config->tseg1 = CAN_BTR_TS1_11TQ;
                    config->tseg2 = CAN_BTR_TS2_2TQ;
                    config->sjw = CAN_BTR_SJW_1TQ;
                }
                break;

            default:
                SS_ERROR("unsupported apb1 clock for can baudrate");
        }
        break;

        default:
            SS_ERROR("unsupported can baudrate");
    }

    return true;
}

bool ss_get_spi_prescaler(uint32_t baudrate, uint32_t clk, uint32_t* prescaler) {

    uint32_t br = clk / baudrate;
    br = (br < 2 || br > 256) ? 2 : br;

    switch(br) {
        case 2: *prescaler = SPI_CR1_BR_FPCLK_DIV_2; break;
        case 4: *prescaler = SPI_CR1_BR_FPCLK_DIV_4; break;
        case 8: *prescaler = SPI_CR1_BR_FPCLK_DIV_8; break;
        case 16: *prescaler = SPI_CR1_BR_FPCLK_DIV_16; break;
        case 32: *prescaler = SPI_CR1_BR_FPCLK_DIV_32; break;
        case 64: *prescaler = SPI_CR1_BR_FPCLK_DIV_64; break;
        case 128: *prescaler = SPI_CR1_BR_FPCLK_DIV_128; break;
        case 256: *prescaler = SPI_CR1_BR_FPCLK_DIV_256; break;

        default:
            *prescaler = SPI_CR1_BR_FPCLK_DIV_256;
            SS_ERROR("unsupported spi prescaler");
    }

    return true;
}

bool ss_clock_spi(uint32_t* prescaler, uint32_t baudrate, uint8_t interface) {

    uint32_t spi_clock_speed = 1000000;

    switch (interface) {
        case 1:
            spi_clock_speed *= ss_clock.apb2;
            break;

        case 2:
        case 3:
            spi_clock_speed *= ss_clock.apb1;
            break;

        default:
            SS_ERROR("unknown spi interface");
    }

    if (!ss_get_spi_prescaler(baudrate, spi_clock_speed, prescaler)) SS_ERROR(NULL);

    return true;
}

bool ss_clock_fm(uint16_t pin_id, uint32_t *frequency) {
    switch (pin_id) {
        case PIN('A', 8):
        case PIN('A', 9):
        case PIN('A', 10):
        case PIN('A', 11): 
        case PIN('C', 6):
        case PIN('C', 7):
        case PIN('C', 8):
        case PIN('C', 9): 

            *frequency = ss_clock.apb2 * 2;
            break;

        case PIN('A', 5):
        case PIN('A', 15):
        case PIN('B', 10):
        case PIN('B', 11): 
        case PIN('A', 6):
        case PIN('A', 7):
        case PIN('B', 0):
        case PIN('B', 1): 
        case PIN('A', 0):
        case PIN('A', 1):
        case PIN('A', 2):
        case PIN('A', 3):
        case PIN('B', 14):
        case PIN('B', 15): 
            *frequency = ss_clock.apb1 * 2;
            break;

        default:
            SS_ERROR("unknown pin_id");
    }

    return true;
}

#endif // COMPILE_SS_CLOCK
