#include <string.h>
#include "ss_simulink_queue.h"

#define SIM_DEPTH 8

struct SIM_ITEM {
    uint32_t len;
    uint8_t data[SS_SIMULINK_QUEUE_MAX_LEN];
};

struct SIM_QUEUE {
    struct SIM_ITEM items[SIM_DEPTH];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
    uint8_t ready;
    uint8_t overrun;
    uint16_t max_len;
};

static struct SIM_QUEUE queues[SS_SIMULINK_QUEUE_COUNT];

bool ss_simulink_queue_init(uint8_t descriptor, uint16_t max_len, uint8_t depth) {
    if (descriptor >= SS_SIMULINK_QUEUE_COUNT) {
        return false;
    }

    queues[descriptor].ready = 1;
    queues[descriptor].max_len = max_len;

    return true;
}

bool ss_simulink_queue_push(uint8_t descriptor, const uint8_t *data, uint32_t length) {
    struct SIM_QUEUE *q;

    if (descriptor >= SS_SIMULINK_QUEUE_COUNT || !queues[descriptor].ready) {
        return false;
    }

    q = &queues[descriptor];

    if (q->count >= SIM_DEPTH) {
        q->overrun = 1;
        return false;
    }

    if (length > q->max_len) {
        length = q->max_len;
    }

    memset(&q->items[q->head], 0, sizeof(struct SIM_ITEM));
    memcpy(q->items[q->head].data, data, length);
    q->items[q->head].len = length;

    q->head = (uint8_t) ((q->head + 1) % SIM_DEPTH);
    q->count++;

    return true;
}

bool ss_simulink_queue_pop(uint8_t descriptor, uint8_t *data, uint32_t max_len,
                           uint32_t *length) {
    struct SIM_QUEUE *q;
    uint32_t copy;

    if (descriptor >= SS_SIMULINK_QUEUE_COUNT || !queues[descriptor].ready) {
        return false;
    }

    q = &queues[descriptor];

    if (q->count == 0) {
        return false;
    }

    copy = (q->items[q->tail].len > max_len) ? max_len : q->items[q->tail].len;

    memcpy(data, q->items[q->tail].data, copy);
    *length = copy;

    q->tail = (uint8_t) ((q->tail + 1) % SIM_DEPTH);
    q->count--;

    return true;
}

void ss_simulink_queue_pop_latest(uint8_t descriptor, uint8_t *data, uint32_t max_len,
                                  uint32_t *length) {
    static uint8_t last[SS_SIMULINK_QUEUE_COUNT][SS_SIMULINK_QUEUE_MAX_LEN];
    static uint32_t last_len[SS_SIMULINK_QUEUE_COUNT];
    uint32_t len;
    uint32_t copy;

    if (descriptor >= SS_SIMULINK_QUEUE_COUNT) {
        *length = 0;
        return;
    }

    if (ss_simulink_queue_pop(descriptor, last[descriptor], SS_SIMULINK_QUEUE_MAX_LEN, &len)) {
        last_len[descriptor] = len;
    }

    copy = (last_len[descriptor] > max_len) ? max_len : last_len[descriptor];

    memcpy(data, last[descriptor], copy);
    *length = copy;
}

uint8_t ss_simulink_queue_overrun(uint8_t descriptor) {
    uint8_t flag;

    if (descriptor >= SS_SIMULINK_QUEUE_COUNT) {
        return 0;
    }

    flag = queues[descriptor].overrun;
    queues[descriptor].overrun = 0;

    return flag;
}
