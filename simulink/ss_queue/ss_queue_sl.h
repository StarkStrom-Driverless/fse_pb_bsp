#ifndef _SS_QUEUE_SL_H_
#define _SS_QUEUE_SL_H_

#include <inttypes.h>
#include "ss_bits.h"
#include "ss_simulink_queue.h"

static inline unsigned int ss_queue_sl_get(const uint8_t *data, uint16_t start_bit,
                                           uint8_t length, uint8_t byte_order) {
    return (unsigned int) ss_bits_get(data, SS_SIMULINK_QUEUE_MAX_LEN, start_bit,
                                      length, byte_order);
}

static inline int ss_queue_sl_get_signed(const uint8_t *data, uint16_t start_bit,
                                         uint8_t length, uint8_t byte_order) {
    return (int) ss_bits_get_signed(data, SS_SIMULINK_QUEUE_MAX_LEN, start_bit,
                                    length, byte_order);
}

static inline void ss_queue_sl_open(uint8_t descriptor, uint16_t width, uint8_t depth) {
    ss_simulink_queue_init(descriptor, width, depth);
}

static inline void ss_queue_sl_pop(uint8_t descriptor, uint16_t width, uint8_t *data,
                                   unsigned int *length, uint8_t *overrun) {
    uint32_t len = 0;
    uint16_t i;

    *overrun = ss_simulink_queue_overrun(descriptor);

    ss_simulink_queue_pop_latest(descriptor, data, width, &len);

    for (i = (uint16_t) len; i < width; i++) {
        data[i] = 0;
    }

    *length = (unsigned int) len;
}

static inline void ss_queue_sl_push(uint8_t descriptor, uint16_t width, const uint8_t *data,
                                    uint32_t length, uint8_t enable, uint8_t *overrun) {
    if (!enable) {
        *overrun = 0;
        return;
    }

    if (length > width) {
        length = width;
    }

    *overrun = ss_simulink_queue_push(descriptor, data, length) ? 0 : 1;
}

#endif
