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
#include <libopencm3/stm32/timer.h>

#define SS_CLOCK_HSI_KHZ        16000u
#define SS_CLOCK_HSE_RTCPRE     31u
#define SS_CLOCK_HSE_ICPSC      8u
#define SS_CLOCK_HSE_SPANS      16u
#define SS_CLOCK_HSE_TOLERANCE  6u
#define SS_CLOCK_HSE_TIMEOUT    4000000u

static const uint16_t ss_clock_hse_candidates[] = { 8, 12, 16, 20, 25 };

struct SS_CLOCK ss_clock;

bool ss_clock_hse(uint16_t* hse_mhz) {

    rcc_osc_on(RCC_HSI);
    for (uint32_t i = 0; !(RCC_CR & RCC_CR_HSIRDY); i++)
        if (i > SS_CLOCK_HSE_TIMEOUT) SS_ERROR("hsi not ready");

    rcc_set_sysclk_source(RCC_CFGR_SW_HSI);
    rcc_set_hpre(RCC_CFGR_HPRE_NODIV);
    rcc_set_ppre1(RCC_CFGR_PPRE_NODIV);
    rcc_set_ppre2(RCC_CFGR_PPRE_NODIV);

    rcc_osc_on(RCC_HSE);
    for (uint32_t i = 0; !(RCC_CR & RCC_CR_HSERDY); i++)
        if (i > SS_CLOCK_HSE_TIMEOUT) SS_ERROR("hse not ready");

    RCC_CFGR = (RCC_CFGR & ~(RCC_CFGR_RTCPRE_MASK << RCC_CFGR_RTCPRE_SHIFT))
             | (SS_CLOCK_HSE_RTCPRE << RCC_CFGR_RTCPRE_SHIFT);

    rcc_periph_clock_enable(RCC_TIM11);
    rcc_periph_reset_pulse(RST_TIM11);

    TIM_OR(TIM11) = 0x2;

    TIM_CR1(TIM11) = 0;
    TIM_PSC(TIM11) = 0;
    TIM_ARR(TIM11) = 0xffff;

    timer_ic_set_input(TIM11, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_filter(TIM11, TIM_IC1, TIM_IC_OFF);
    timer_ic_set_prescaler(TIM11, TIM_IC1, TIM_IC_PSC_8);
    timer_ic_enable(TIM11, TIM_IC1);
    timer_enable_counter(TIM11);

    uint16_t first = 0;
    uint16_t last = 0;

    for (uint32_t n = 0; n <= SS_CLOCK_HSE_SPANS; n++) {

        TIM_SR(TIM11) = ~TIM_SR_CC1IF;

        for (uint32_t i = 0; !(TIM_SR(TIM11) & TIM_SR_CC1IF); i++)
            if (i > SS_CLOCK_HSE_TIMEOUT) SS_ERROR("no hse_rtc edges on tim11");

        last = (uint16_t)TIM_CCR1(TIM11);
        if (n == 0) first = last;
    }

    timer_disable_counter(TIM11);
    timer_ic_disable(TIM11, TIM_IC1);
    rcc_periph_reset_pulse(RST_TIM11);
    rcc_periph_clock_disable(RCC_TIM11);

    uint32_t delta = (uint16_t)(last - first);
    if (delta == 0) SS_ERROR("hse measurement underflow");

    uint32_t measured = (SS_CLOCK_HSI_KHZ * SS_CLOCK_HSE_RTCPRE
                        * SS_CLOCK_HSE_ICPSC * SS_CLOCK_HSE_SPANS) / delta;

    for (uint32_t i = 0; i < sizeof(ss_clock_hse_candidates) / sizeof(ss_clock_hse_candidates[0]); i++) {

        uint32_t nominal = ss_clock_hse_candidates[i] * 1000u;
        uint32_t window = (nominal * SS_CLOCK_HSE_TOLERANCE) / 100u;

        if (measured + window >= nominal && measured <= nominal + window) {
            *hse_mhz = ss_clock_hse_candidates[i];
            return true;
        }
    }

    SS_ERROR("hse frequency does not match any known crystal");
}

bool ss_clock_init(uint8_t config) {

    ss_clock.ahb = 16;
    ss_clock.apb1 = 16;
    ss_clock.apb2 = 16;
    ss_clock.hse = 0;

    switch (config)
    {
        case SS_CLOCK_DEFAULT:
        break;

        case SS_CLOCK_FAST:
            {
                uint16_t hse = 0;
                if (!ss_clock_hse(&hse)) SS_ERROR(NULL);

                struct rcc_clock_scale scale = rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ];
                scale.pllm = (hse % 2u) ? (uint8_t)hse : (uint8_t)(hse / 2u);
                scale.plln = (hse % 2u) ? 336 : 168;
                scale.pllp = 2;
                scale.pllq = 7;

                ss_clock.hse = hse;
                ss_clock.ahb = 168;
                ss_clock.apb1 = 42;
                ss_clock.apb2 = 84;
                rcc_clock_setup_pll(&scale);
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
