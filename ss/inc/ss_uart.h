/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_UART

#ifndef _SS_UART_H_
#define _SS_UART_H_

#include "ss_feedback.h"

#include <inttypes.h>
#include <stdbool.h>

#include <FreeRTOS.h>
#include <queue.h>

#define SS_UART_CHANNEL_CNT 6

struct SS_UART_CHANNEL_QUEUE {
    QueueHandle_t queue;
    bool enabled;
    volatile bool tx_busy;
};

struct SS_UART_CHANNEL {
    struct SS_UART_CHANNEL_QUEUE rx;
    struct SS_UART_CHANNEL_QUEUE tx;
};

struct SS_UART {
    struct SS_UART_CHANNEL channels[SS_UART_CHANNEL_CNT];
};
extern struct SS_UART ss_uart;


#ifdef USE_PRIVATE
void ss_usart_irq_generic(uint8_t interface);
#endif

#ifdef USE_PRIVATE
bool ss_uart_set_rcc_from_interface(uint8_t interface);
#endif

#ifdef USE_PRIVATE
bool ss_uart_get_af_mode_from_interface(uint8_t interface, uint32_t* af);
#endif

#ifdef USE_PRIVATE
bool ss_uart_get_pins_from_interface(uint8_t interface, uint16_t* rx, uint16_t* tx);
#endif

#ifdef USE_PRIVATE
bool ss_uart_get_uart_addr_from_interface(uint8_t interface, uint32_t* uart_addr);
#endif

#ifdef USE_PRIVATE
bool ss_uart_get_nvic_irq_from_interface(uint8_t interface, uint32_t* irq);
#endif

#ifdef USE_PRIVATE
void ss_uart_queue_init();
#endif

#ifdef USE_PRIVATE
bool ss_uart_queue_channel_get(uint8_t interface, struct SS_UART_CHANNEL** queue);
#endif

#ifdef USE_PRIVATE
bool ss_uart_queue_add(uint8_t interface, uint32_t depth);
#endif


bool ss_uart_init(uint8_t interface, uint32_t baudrate);
bool ss_uart_send(uint8_t interface, uint8_t* value, uint32_t len);
bool ss_uart_flush(uint8_t interface);
bool ss_uart_send_str(uint8_t interface, char* str);
bool ss_uart_read(uint8_t interface, uint8_t* data);

#endif

#endif