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


#include "ss_gpio.h"
#include "ss_makros.h"
#include "ss_leds.h"
#include "ss_uart.h"

#include <string.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#define SS_FEEDBACK_BASE                            SS_FEEDBACK_BASE_NOT_SET

#if COMPILE_SS_UART


/*
*   UART ISR's
*/

void usart1_isr(void) {
    ss_usart_irq_generic(1);
}

void usart2_isr(void) {
    ss_usart_irq_generic(2);
}

void usart3_isr(void) {
    ss_usart_irq_generic(3);
}

void uart4_isr(void) {
    ss_usart_irq_generic(4);
}



void usart6_isr(void) {
    ss_usart_irq_generic(6);
}


void ss_usart_irq_generic(uint8_t interface) {
    uint32_t uart_addr = 0;

    ss_uart_get_uart_addr_from_interface(interface, &uart_addr);

    if (usart_get_flag(uart_addr, USART_SR_RXNE)) {
        uint8_t byte = usart_recv(uart_addr);

        ss_led_dbg2_toggle();
    }
}

/**
 * 
 */
SS_FEEDBACK ss_uart_set_rcc_from_interface(uint8_t interface) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    switch (interface) {
        case 1: rcc_periph_clock_enable(RCC_USART1); break;
        case 2: rcc_periph_clock_enable(RCC_USART2); break;
        case 3: rcc_periph_clock_enable(RCC_USART3); break;
        case 4: rcc_periph_clock_enable(RCC_UART4);  break;
        case 6: rcc_periph_clock_enable(RCC_USART6); break;
        default: rc = SS_FEEDBACK_ERROR; break;
    }

    return rc;
}

SS_FEEDBACK ss_uart_get_af_mode_from_interface(uint8_t interface, uint32_t* af) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    switch (interface) {
        case 1: *af = GPIO_AF1; break;

        case 3:
        case 2: *af = GPIO_AF7; break;

        case 6:
        case 4: *af = GPIO_AF8; break;

        default: rc = SS_FEEDBACK_ERROR; break;
    }

    return rc;
}


SS_FEEDBACK ss_uart_get_pins_from_interface(uint8_t interface, uint16_t* rx, uint16_t* tx) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    switch (interface) {
        case 1: *rx = PIN('A', 10); *tx = PIN('A', 9); break;
        case 2: *rx = PIN('A', 3); *tx = PIN('A', 2); break;
        case 3: *rx = PIN('B', 11); *tx = PIN('B', 10); break;
        case 4: *rx = PIN('A', 1); *tx = PIN('A', 0); break;
        case 6: *rx = PIN('C', 7); *tx = PIN('C', 6); break;
        default: rc = SS_FEEDBACK_ERROR; break;
    }
        
    return rc;
}

SS_FEEDBACK ss_uart_get_uart_addr_from_interface(uint8_t interface, uint32_t* uart_addr) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    switch (interface) {
        case 1: *uart_addr = USART1; break;
        case 2: *uart_addr = USART2; break;
        case 3: *uart_addr = USART3; break;
        case 4: *uart_addr = UART4 ; break;
        case 6: *uart_addr = USART6; break;
        default: rc = SS_FEEDBACK_ERROR; break;
    }
        
    return rc;
}

SS_FEEDBACK ss_uart_get_nvic_irq_from_interface(uint8_t interface, uint32_t* irq) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    switch (interface) {
        case 1: *irq = NVIC_USART1_IRQ; break;
        case 2: *irq = NVIC_USART2_IRQ; break;
        case 3: *irq = NVIC_USART3_IRQ; break;
        case 4: *irq = NVIC_UART4_IRQ; break;
        case 6: *irq = NVIC_USART6_IRQ; break;
        default: rc = SS_FEEDBACK_ERROR; break;
    }
        
    return rc;
}

/**
 * USER FUNCTION
 */

SS_FEEDBACK ss_uart_init(uint8_t interface, uint32_t baudrate) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    if (interface == 0) rc = SS_FEEDBACK_ERROR;
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    uint16_t rx, tx;
    uint32_t af;
    uint32_t uart_addr;
    uint32_t uart_nvic;

    rc = ss_uart_set_rcc_from_interface(interface);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_uart_get_af_mode_from_interface(interface, &af);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_uart_get_pins_from_interface(interface, &rx, &tx);
    SS_HANDLE_ERROR_WITH_EXIT(rc);


    ss_enable_rcc_from_id(rx);
    gpio_mode_setup(GPIO(PINBANK(rx)), GPIO_MODE_AF, GPIO_PUPD_NONE, BIT(PINNO(rx)));
    gpio_set_af(GPIO(PINBANK(rx)), af, BIT(PINNO(rx)));
    gpio_set_output_options(GPIO(PINBANK(rx)), GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BIT(PINNO(rx)));
    

    ss_enable_rcc_from_id(tx);
    gpio_mode_setup(GPIO(PINBANK(tx)), GPIO_MODE_AF, GPIO_PUPD_NONE, BIT(PINNO(tx)));
    gpio_set_af(GPIO(PINBANK(tx)), af, BIT(PINNO(tx)));
    gpio_set_output_options(GPIO(PINBANK(tx)), GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BIT(PINNO(tx)));
    


    rc = ss_uart_get_uart_addr_from_interface(interface, &uart_addr);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    usart_set_baudrate(uart_addr, baudrate);
    usart_set_databits(uart_addr, 8);
    usart_set_stopbits(uart_addr, USART_STOPBITS_1);
    usart_set_parity(uart_addr, USART_PARITY_NONE);
    usart_set_flow_control(uart_addr, USART_FLOWCONTROL_NONE);
    usart_set_mode(uart_addr, USART_MODE_TX_RX);


    rc = ss_uart_get_nvic_irq_from_interface(interface, &uart_nvic);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    usart_enable_rx_interrupt(uart_addr);
    nvic_enable_irq(uart_nvic);

    usart_enable(uart_addr);

    return rc;
}

SS_FEEDBACK ss_uart_send_str(uint8_t interface, char* str) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    rc = ss_uart_send(interface, (uint8_t*)str, strlen(str)+1);

    return rc;
}

SS_FEEDBACK ss_uart_send(uint8_t interface, uint8_t* value, uint32_t len) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    uint32_t uart_addr = 0;

    rc = ss_uart_get_uart_addr_from_interface(interface, &uart_addr);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    for (uint32_t i = 0; i < len; i++) {
        usart_send_blocking(uart_addr, value[i]);
    }

    return rc;
}

#endif