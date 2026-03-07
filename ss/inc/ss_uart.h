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

#ifdef USE_PRIVATE
void ss_usart_irq_generic(uint8_t interface);
#endif

#ifdef USE_PRIVATE
SS_FEEDBACK ss_uart_set_rcc_from_interface(uint8_t interface);
#endif

#ifdef USE_PRIVATE
SS_FEEDBACK ss_uart_get_af_mode_from_interface(uint8_t interface, uint32_t* af);
#endif

#ifdef USE_PRIVATE
SS_FEEDBACK ss_uart_get_pins_from_interface(uint8_t interface, uint16_t* rx, uint16_t* tx);
#endif

#ifdef USE_PRIVATE
SS_FEEDBACK ss_uart_get_uart_addr_from_interface(uint8_t interface, uint32_t* uart_addr);
#endif

#ifdef USE_PRIVATE
SS_FEEDBACK ss_uart_get_nvic_irq_from_interface(uint8_t interface, uint32_t* irq);
#endif


SS_FEEDBACK ss_uart_init(uint8_t interface, uint32_t baudrate);
SS_FEEDBACK ss_uart_send_str(uint8_t interface, char* str);
SS_FEEDBACK ss_uart_send(uint8_t interface, uint8_t* value, uint32_t len);

#endif

#endif