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

#if COMPILE_SS_ERROR
#include "ss_error.h"

#include <FreeRTOS.h>
#include <task.h>
#include <stddef.h>


static struct SS_ERROR_CONTEXT ss_boot_error_ctx;

static struct SS_ERROR_CONTEXT* ss_error_current_ctx(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return &ss_boot_error_ctx;
    }

    struct SS_ERROR_CONTEXT *ctx = (struct SS_ERROR_CONTEXT*)
        pvTaskGetThreadLocalStoragePointer(NULL, SS_ERROR_TLS_INDEX);

    return (ctx != NULL) ? ctx : &ss_boot_error_ctx;
}

void ss_error_register_task(void *task_handle, struct SS_ERROR_CONTEXT *ctx) {
    ctx->depth = 0;
    vTaskSetThreadLocalStoragePointer((TaskHandle_t)task_handle, SS_ERROR_TLS_INDEX, ctx);
}

void ss_error_push(const char *func, uint16_t line, const char *msg) {
    struct SS_ERROR_CONTEXT *ctx = ss_error_current_ctx();

    if (ctx->depth < SS_ERROR_STACK_DEPTH) {
        ctx->frames[ctx->depth].func = func;
        ctx->frames[ctx->depth].line = line;
        ctx->frames[ctx->depth].msg  = msg;
        ctx->depth++;
    }
}

void ss_error_reset(void) {
    ss_error_current_ctx()->depth = 0;
}

static void ss_error_append_char(char *buf, uint32_t *pos, uint32_t cap, char c) {
    if (*pos < cap - 1) {
        buf[(*pos)++] = c;
    }
}

static void ss_error_append_str(char *buf, uint32_t *pos, uint32_t cap, const char *s) {
    while (*s != '\0') {
        ss_error_append_char(buf, pos, cap, *s++);
    }
}

static void ss_error_append_uint(char *buf, uint32_t *pos, uint32_t cap, uint32_t val) {
    char digits[10];
    uint8_t n = 0;

    if (val == 0) {
        ss_error_append_char(buf, pos, cap, '0');
        return;
    }

    while (val > 0) {
        digits[n++] = '0' + (val % 10);
        val /= 10;
    }

    while (n > 0) {
        ss_error_append_char(buf, pos, cap, digits[--n]);
    }
}

static void ss_error_format_frame(char *buf, uint32_t cap, int n, const struct SS_ERROR_FRAME *f) {
    uint32_t pos = 0;

    ss_error_append_uint(buf, &pos, cap, (uint32_t)n);
    ss_error_append_str(buf, &pos, cap, ": ");
    ss_error_append_str(buf, &pos, cap, f->func);
    ss_error_append_str(buf, &pos, cap, "() line ");
    ss_error_append_uint(buf, &pos, cap, f->line);

    if (f->msg != NULL) {
        ss_error_append_str(buf, &pos, cap, " -> ");
        ss_error_append_str(buf, &pos, cap, f->msg);
    }

    ss_error_append_str(buf, &pos, cap, "\r\n");
    buf[pos] = '\0';
}

static const char SS_ERROR_TRACE_HEADER[] = "---- SS_ERROR_ASSERT ---\r\n";

void ss_error_dump(void (*writer)(const char *str)) {
    struct SS_ERROR_CONTEXT *ctx = ss_error_current_ctx();

    if (writer != NULL) {
        writer(SS_ERROR_TRACE_HEADER);

        for (int i = (int)ctx->depth - 1, n = 1; i >= 0; i--, n++) {
            char line[80];
            ss_error_format_frame(line, sizeof(line), n, &ctx->frames[i]);
            writer(line);
        }
    }

    ss_error_reset();
}

#if COMPILE_SS_UART
#include "ss_uart.h"
#include <libopencm3/stm32/usart.h>

static void ss_error_write_blocking(uint8_t uart_interface, const char *str) {
    uint32_t uart_addr = 0;

    if (!ss_uart_get_uart_addr_from_interface(uart_interface, &uart_addr)) {
        return;
    }

    while (*str != '\0') {
        usart_send_blocking(uart_addr, (uint8_t)*str++);
    }
}

void ss_error_print_trace(uint8_t uart_interface) {
    struct SS_ERROR_CONTEXT *ctx = ss_error_current_ctx();

    ss_error_write_blocking(uart_interface, SS_ERROR_TRACE_HEADER);

    for (int i = (int)ctx->depth - 1, n = 1; i >= 0; i--, n++) {
        char line[80];
        ss_error_format_frame(line, sizeof(line), n, &ctx->frames[i]);
        ss_error_write_blocking(uart_interface, line);
    }

    ss_error_reset();
}
#endif // COMPILE_SS_UART

#endif // COMPILE_SS_ERROR
