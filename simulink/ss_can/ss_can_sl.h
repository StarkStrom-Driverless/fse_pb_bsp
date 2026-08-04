#ifndef _SS_CAN_SL_H_
#define _SS_CAN_SL_H_

#include <inttypes.h>
#include "ss_can.h"

static inline void ss_can_sl_open(void **queue, uint8_t channel, uint32_t id) {
    ss_can_queue_add(channel, id, (struct SS_CAN_MSG_QUEUE **) queue);
}

static inline void ss_can_sl_read(void *queue, uint8_t *data, uint8_t *dlc, uint8_t *valid) {
    struct SS_CAN_FRAME frame;
    uint8_t i;

    if (queue == 0 || !ss_can_queue_read((struct SS_CAN_MSG_QUEUE *) queue, &frame)) {
        for (i = 0; i < 8; i++) {
            data[i] = 0;
        }

        *dlc = 0;
        *valid = 0;
        return;
    }

    for (i = 0; i < 8; i++) {
        data[i] = frame.data[i];
    }

    *dlc = frame.dlc;
    *valid = 1;
}

static inline uint32_t ss_can_sl_get_signal(const uint8_t *data, uint8_t start_bit, uint8_t length) {
    struct SS_CAN_FRAME frame;
    uint8_t i;

    for (i = 0; i < 8; i++) {
        frame.data[i] = data[i];
    }

    return (uint32_t) ss_can_frame_get_signal(&frame, start_bit, length);
}

static inline int32_t ss_can_sl_get_signal_signed(const uint8_t *data, uint8_t start_bit,
                                                  uint8_t length) {
    uint32_t raw = ss_can_sl_get_signal(data, start_bit, length);

    if (length >= 32) {
        return (int32_t) raw;
    }

    if (raw & (1UL << (length - 1))) {
        return (int32_t) (raw | (~0UL << length));
    }

    return (int32_t) raw;
}

static inline void ss_can_sl_set_signal(const uint8_t *data_in, uint8_t start_bit,
                                        uint8_t length, uint32_t value, uint8_t *data_out) {
    struct SS_CAN_FRAME frame;
    uint8_t i;

    for (i = 0; i < 8; i++) {
        frame.data[i] = data_in[i];
    }

    ss_can_frame_set_signal(&frame, start_bit, length, (uint64_t) value);

    for (i = 0; i < 8; i++) {
        data_out[i] = frame.data[i];
    }
}

static inline void ss_can_sl_send(uint8_t channel, uint32_t id, uint8_t dlc, const uint8_t *data) {
    struct SS_CAN_FRAME frame;
    uint8_t i;

    ss_can_frame_reset(&frame);
    ss_can_frame_set_common(&frame, id, dlc);

    for (i = 0; i < 8; i++) {
        frame.data[i] = data[i];
    }

    ss_can_send(channel, &frame);
}

#endif
