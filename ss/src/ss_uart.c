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
#include "ss_error.h"

#include <string.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>


#if COMPILE_SS_UART

struct SS_UART ss_uart = {0};

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
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t uart_addr = 0;

    ss_uart_get_uart_addr_from_interface(interface, &uart_addr);

    struct SS_UART_CHANNEL* channel;

    if (usart_get_flag(uart_addr, USART_SR_RXNE)) {
        uint8_t byte = usart_recv(uart_addr);

        if (ss_uart_queue_channel_get(interface, &channel)) {
            xQueueSendFromISR(channel->rx.queue, &byte, &xHigherPriorityTaskWoken);
        }
    }

    if (usart_get_flag(uart_addr, USART_SR_TC)) {
        if (ss_uart_queue_channel_get(interface, &channel)) {
            uint8_t byte;
            if (xQueueReceiveFromISR(channel->tx.queue, &byte, &xHigherPriorityTaskWoken) == pdPASS) {
                usart_send(uart_addr, byte);
            } else {
                channel->tx.tx_busy = false;
                usart_disable_tx_complete_interrupt(uart_addr);
            }
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * 
 */
bool ss_uart_set_rcc_from_interface(uint8_t interface) {
    switch (interface) {
        case 1: rcc_periph_clock_enable(RCC_USART1); break;
        case 2: rcc_periph_clock_enable(RCC_USART2); break;
        case 3: rcc_periph_clock_enable(RCC_USART3); break;
        case 4: rcc_periph_clock_enable(RCC_UART4);  break;
        case 6: rcc_periph_clock_enable(RCC_USART6); break;
        default: SS_ERROR("unknown uart interface");
    }

    return true;
}

bool ss_uart_get_af_mode_from_interface(uint8_t interface, uint32_t* af) {
    switch (interface) {
        case 1: *af = GPIO_AF1; break;

        case 3:
        case 2: *af = GPIO_AF7; break;

        case 6:
        case 4: *af = GPIO_AF8; break;

        default: SS_ERROR("unknown uart interface");
    }

    return true;
}


bool ss_uart_get_pins_from_interface(uint8_t interface, uint16_t* rx, uint16_t* tx) {
    switch (interface) {
        case 1: *rx = PIN('A', 10); *tx = PIN('A', 9); break;
        case 2: *rx = PIN('A', 3); *tx = PIN('A', 2); break;
        case 3: *rx = PIN('B', 11); *tx = PIN('B', 10); break;
        case 4: *rx = PIN('C', 11); *tx = PIN('C', 10); break;
        case 6: *rx = PIN('C', 7); *tx = PIN('C', 6); break;
        default: SS_ERROR("unknown uart interface");
    }

    return true;
}

bool ss_uart_get_uart_addr_from_interface(uint8_t interface, uint32_t* uart_addr) {
    switch (interface) {
        case 1: *uart_addr = USART1; break;
        case 2: *uart_addr = USART2; break;
        case 3: *uart_addr = USART3; break;
        case 4: *uart_addr = UART4 ; break;
        case 6: *uart_addr = USART6; break;
        default: return false;
    }

    return true;
}

bool ss_uart_get_nvic_irq_from_interface(uint8_t interface, uint32_t* irq) {
    switch (interface) {
        case 1: *irq = NVIC_USART1_IRQ; break;
        case 2: *irq = NVIC_USART2_IRQ; break;
        case 3: *irq = NVIC_USART3_IRQ; break;
        case 4: *irq = NVIC_UART4_IRQ; break;
        case 6: *irq = NVIC_USART6_IRQ; break;
        default: SS_ERROR("unknown uart interface");
    }

    return true;
}

/**
 * Free RTOS Stuff
 */

void ss_uart_queue_init() {
    static uint8_t init = 0;
    if (init == 1) {
        return; 
    }

    for (int i = 0; i < SS_UART_CHANNEL_CNT; i++) {
        ss_uart.channels[i].rx.enabled = false;
        ss_uart.channels[i].tx.enabled = false;
    }

    init = 1;
}

bool ss_uart_queue_channel_get(uint8_t interface, struct SS_UART_CHANNEL** queue) {
    if (interface == 0 || interface >= 7) {
        return false;
    }

    *queue = &ss_uart.channels[interface-1];

    return true;
}

bool ss_uart_queue_add(uint8_t interface, uint32_t depth) {
    struct SS_UART_CHANNEL* channel;

    if (!ss_uart_queue_channel_get(interface, &channel)) SS_ERROR(NULL);

    channel->tx.queue = xQueueCreate(depth, sizeof(uint8_t));
    channel->tx.enabled = true;

    channel->rx.queue = xQueueCreate(depth, sizeof(uint8_t));
    channel->rx.enabled = true;

    return true;
}




/**
 * USER FUNCTION
 */

bool ss_uart_init(uint8_t interface, uint32_t baudrate) {
    if (interface == 0) SS_ERROR("unknown uart interface");

    uint16_t rx, tx;
    uint32_t af;
    uint32_t uart_addr;
    uint32_t uart_nvic;

    if (!ss_uart_set_rcc_from_interface(interface)) SS_ERROR(NULL);

    if (!ss_uart_get_af_mode_from_interface(interface, &af)) SS_ERROR(NULL);

    if (!ss_uart_get_pins_from_interface(interface, &rx, &tx)) SS_ERROR(NULL);


    ss_enable_rcc_from_id(rx);
    gpio_mode_setup(GPIO(PINBANK(rx)), GPIO_MODE_AF, GPIO_PUPD_NONE, BIT(PINNO(rx)));
    gpio_set_af(GPIO(PINBANK(rx)), af, BIT(PINNO(rx)));
    gpio_set_output_options(GPIO(PINBANK(rx)), GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BIT(PINNO(rx)));


    ss_enable_rcc_from_id(tx);
    gpio_mode_setup(GPIO(PINBANK(tx)), GPIO_MODE_AF, GPIO_PUPD_NONE, BIT(PINNO(tx)));
    gpio_set_af(GPIO(PINBANK(tx)), af, BIT(PINNO(tx)));
    gpio_set_output_options(GPIO(PINBANK(tx)), GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BIT(PINNO(tx)));



    if (!ss_uart_get_uart_addr_from_interface(interface, &uart_addr)) SS_ERROR("unknown uart interface");

    usart_set_baudrate(uart_addr, baudrate);
    usart_set_databits(uart_addr, 8);
    usart_set_stopbits(uart_addr, USART_STOPBITS_1);
    usart_set_parity(uart_addr, USART_PARITY_NONE);
    usart_set_flow_control(uart_addr, USART_FLOWCONTROL_NONE);
    usart_set_mode(uart_addr, USART_MODE_TX_RX);


    if (!ss_uart_get_nvic_irq_from_interface(interface, &uart_nvic)) SS_ERROR(NULL);

    usart_enable_rx_interrupt(uart_addr);
    nvic_set_priority(uart_nvic, configMAX_SYSCALL_INTERRUPT_PRIORITY);
    nvic_enable_irq(uart_nvic);

    usart_enable(uart_addr);

    ss_uart_queue_init();
    if (!ss_uart_queue_add(interface, 8000)) SS_ERROR(NULL);

    return true;
}

bool ss_uart_send(uint8_t interface, uint8_t* value, uint32_t len) {
    struct SS_UART_CHANNEL* channel;

    if (!ss_uart_queue_channel_get(interface, &channel)) SS_ERROR(NULL);

    if (!channel->tx.enabled) {
        SS_ERROR("uart tx channel not enabled");
    }

    for (uint32_t i = 0; i < len; i++) {
        xQueueSend(channel->tx.queue, &value[i], portMAX_DELAY);
    }

    return true;
}

bool ss_uart_flush(uint8_t interface) {
    uint32_t uart_addr = 0;
    struct SS_UART_CHANNEL* channel;

    if (!ss_uart_get_uart_addr_from_interface(interface, &uart_addr)) SS_ERROR("unknown uart interface");

    if (!ss_uart_queue_channel_get(interface, &channel)) SS_ERROR(NULL);

    if (!channel->tx.enabled) {
        SS_ERROR("uart tx channel not enabled");
    }

    if (!channel->tx.tx_busy) {
        uint8_t byte;
        if (xQueueReceive(channel->tx.queue, &byte, 0) == pdPASS) {
            channel->tx.tx_busy = true;
            usart_send(uart_addr, byte);
            usart_enable_tx_complete_interrupt(uart_addr);
        }
    }

    return true;
}

bool ss_uart_send_str(uint8_t interface, char* str) {
    if (!ss_uart_send(interface, (uint8_t*)str, strlen(str))) SS_ERROR(NULL);

    return ss_uart_flush(interface);
}

bool ss_uart_read(uint8_t interface, uint8_t* data) {
    struct SS_UART_CHANNEL* channel;

    if (!ss_uart_queue_channel_get(interface, &channel)) SS_ERROR(NULL);

    if (!channel->rx.enabled) {
        return false;
    }

    return xQueueReceive(channel->rx.queue, data, (TickType_t) 0) == pdPASS;
}

#endif