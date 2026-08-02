/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_ERROR

#ifndef _SS_ERROR_H_
#define _SS_ERROR_H_

#include <inttypes.h>
#include <stdbool.h>

#define SS_ERROR_STACK_DEPTH 8
#define SS_ERROR_TLS_INDEX   0

#ifndef SS_ERROR_UART_INTERFACE
#define SS_ERROR_UART_INTERFACE 4
#endif

struct SS_ERROR_FRAME {
    const char *func;
    uint16_t    line;
    const char *msg;
};

struct SS_ERROR_CONTEXT {
    struct SS_ERROR_FRAME frames[SS_ERROR_STACK_DEPTH];
    uint8_t                depth;
};

void ss_error_push(const char *func, uint16_t line, const char *msg);
void ss_error_reset(void);
void ss_error_dump(void (*writer)(const char *str));

/* Called once per task (from ss_rtos_task_add) so every task gets its own
 * error stack and pushes never need a lock. */
void ss_error_register_task(void *task_handle, struct SS_ERROR_CONTEXT *ctx);

/* Implemented by the application (mirrors ss_init_error()): whatever should
 * happen after the trace has been printed, e.g. blink an LED forever. */
extern void ss_error_fail(void);

#if COMPILE_SS_UART
/* Renders the current task's error stack directly over UART, byte by byte
 * with usart_send_blocking() - deliberately NOT going through ss_uart's
 * queue+interrupt path, since that requires the FreeRTOS scheduler (and its
 * ISR-driven continuation) to already be running. This must work no matter
 * whether the scheduler has started yet, so it only needs the peripheral
 * itself (via ss_uart_init()) to be configured. */
void ss_error_print_trace(uint8_t uart_interface);
#define SS_ERROR_PRINT_TRACE() ss_error_print_trace(SS_ERROR_UART_INTERFACE)
#else
#define SS_ERROR_PRINT_TRACE() ((void)0)
#endif

#define SS_ERROR(msg)                                      \
    do {                                                   \
        ss_error_push(__func__, __LINE__, (msg));           \
        return false;                                       \
    } while (0)

#define SS_ERROR_ASSERT(expr)                               \
    do {                                                     \
        if (!(expr)) {                                        \
            ss_error_push(__func__, __LINE__, NULL);           \
            SS_ERROR_PRINT_TRACE();                             \
            ss_error_fail();                                   \
        }                                                      \
    } while (0)

#endif // _SS_ERROR_H_

#endif // COMPILE_SS_ERROR
