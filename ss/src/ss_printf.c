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

#if COMPILE_SS_PRINTF

#include "ss_printf.h"
#include "ss_uart.h"

#include <stdarg.h>
#include <stdint.h>

#define SS_PRINTF_BUF_SIZE 256

static void append_char(char* buf, uint32_t* pos, char c) {
    if (*pos < SS_PRINTF_BUF_SIZE - 1) {
        buf[(*pos)++] = c;
    }
}

static void append_str(char* buf, uint32_t* pos, const char* s) {
    while (*s) {
        append_char(buf, pos, *s++);
    }
}

static void fmt_uint(char* buf, uint32_t* pos, uint32_t val, uint8_t base) {
    char tmp[32];
    uint8_t i = 0;
    if (val == 0) {
        append_char(buf, pos, '0');
        return;
    }
    while (val > 0) {
        uint8_t digit = val % base;
        tmp[i++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
        val /= base;
    }
    while (i > 0) {
        append_char(buf, pos, tmp[--i]);
    }
}

static void fmt_int(char* buf, uint32_t* pos, int32_t val) {
    if (val < 0) {
        append_char(buf, pos, '-');
        val = -val;
    }
    fmt_uint(buf, pos, (uint32_t)val, 10);
}

static void fmt_float(char* buf, uint32_t* pos, double val) {
    if (val < 0.0) {
        append_char(buf, pos, '-');
        val = -val;
    }
    uint32_t int_part = (uint32_t)val;
    uint32_t frac_part = (uint32_t)((val - (double)int_part) * 1000000.0 + 0.5);
    fmt_uint(buf, pos, int_part, 10);
    append_char(buf, pos, '.');
    /* output exactly 6 decimal digits with leading zeros */
    char frac_str[7];
    uint8_t i;
    frac_str[6] = '\0';
    for (i = 6; i > 0; i--) {
        frac_str[i - 1] = '0' + (frac_part % 10);
        frac_part /= 10;
    }
    append_str(buf, pos, frac_str);
}

static void fmt_binary(char* buf, uint32_t* pos, uint32_t val) {
    char tmp[32];
    uint8_t i = 0;
    if (val == 0) {
        append_char(buf, pos, '0');
        return;
    }
    while (val > 0) {
        tmp[i++] = (val & 1) ? '1' : '0';
        val >>= 1;
    }
    while (i > 0) {
        append_char(buf, pos, tmp[--i]);
    }
}

void ss_printf(uint8_t uart_interface, const char* fmt, ...) {
    char buf[SS_PRINTF_BUF_SIZE];
    uint32_t pos = 0;
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            append_char(buf, &pos, *fmt++);
            continue;
        }
        fmt++;
        switch (*fmt) {
            case 'd': fmt_int(buf, &pos, va_arg(args, int));                 break;
            case 'x': fmt_uint(buf, &pos, va_arg(args, unsigned int), 16);  break;
            case 'f': fmt_float(buf, &pos, va_arg(args, double));            break;
            case 's': append_str(buf, &pos, va_arg(args, const char*));      break;
            case 'c': append_char(buf, &pos, (char)va_arg(args, int));       break;
            case 'b': fmt_binary(buf, &pos, va_arg(args, unsigned int));     break;
            case '%': append_char(buf, &pos, '%');                           break;
            default:  append_char(buf, &pos, '%');
                      append_char(buf, &pos, *fmt);                          break;
        }
        fmt++;
    }

    va_end(args);
    buf[pos] = '\0';
    ss_uart_send_str(uart_interface, buf);
}

#endif // COMPILE_SS_PRINTF
