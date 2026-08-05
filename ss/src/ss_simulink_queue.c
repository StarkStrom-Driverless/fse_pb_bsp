/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_simulink_queue.h"

#if COMPILE_SS_SIMULINK_QUEUE

#include <string.h>
#include <FreeRTOS.h>
#include <queue.h>
#include "ss_error.h"

struct SS_SIMULINK_QUEUE_ITEM {
    uint32_t len;
    uint8_t data[SS_SIMULINK_QUEUE_MAX_LEN];
};

struct SS_SIMULINK_QUEUE {
    QueueHandle_t handle;
    uint16_t max_len;
    uint8_t overrun;
    uint32_t last_len;
    uint8_t last[SS_SIMULINK_QUEUE_MAX_LEN];
};

static struct SS_SIMULINK_QUEUE queues[SS_SIMULINK_QUEUE_COUNT];

bool ss_simulink_queue_init(uint8_t descriptor, uint16_t max_len, uint8_t depth) {
    if (descriptor >= SS_SIMULINK_QUEUE_COUNT) {
        SS_ERROR("simulink queue descriptor out of range");
    }

    if (max_len == 0 || max_len > SS_SIMULINK_QUEUE_MAX_LEN) {
        SS_ERROR("simulink queue payload exceeds SS_SIMULINK_QUEUE_MAX_LEN");
    }

    if (queues[descriptor].handle != NULL) {
        return true;
    }

    if (depth == 0) {
        depth = SS_SIMULINK_QUEUE_DEPTH;
    }

    queues[descriptor].handle = xQueueCreate(depth, sizeof(struct SS_SIMULINK_QUEUE_ITEM));
    if (queues[descriptor].handle == NULL) {
        SS_ERROR("simulink queue create failed");
    }

    queues[descriptor].max_len = max_len;
    queues[descriptor].overrun = 0;

    return true;
}

bool ss_simulink_queue_push(uint8_t descriptor, const uint8_t *data, uint32_t length) {
    struct SS_SIMULINK_QUEUE_ITEM item;

    if (descriptor >= SS_SIMULINK_QUEUE_COUNT || queues[descriptor].handle == NULL) {
        return false;
    }

    if (length > queues[descriptor].max_len) {
        length = queues[descriptor].max_len;
    }

    memset(&item, 0, sizeof(item));
    memcpy(item.data, data, length);
    item.len = length;

    if (xQueueSend(queues[descriptor].handle, &item, (TickType_t) 0) != pdPASS) {
        queues[descriptor].overrun = 1;
        return false;
    }

    return true;
}

bool ss_simulink_queue_pop(uint8_t descriptor, uint8_t *data, uint32_t max_len,
                           uint32_t *length) {
    struct SS_SIMULINK_QUEUE_ITEM item;
    uint32_t copy;

    if (descriptor >= SS_SIMULINK_QUEUE_COUNT || queues[descriptor].handle == NULL) {
        return false;
    }

    if (xQueueReceive(queues[descriptor].handle, &item, (TickType_t) 0) != pdPASS) {
        return false;
    }

    copy = (item.len > max_len) ? max_len : item.len;

    memcpy(data, item.data, copy);
    *length = copy;

    return true;
}

void ss_simulink_queue_pop_latest(uint8_t descriptor, uint8_t *data, uint32_t max_len,
                                  uint32_t *length) {
    struct SS_SIMULINK_QUEUE *q;
    uint32_t copy;
    uint32_t len;

    if (descriptor >= SS_SIMULINK_QUEUE_COUNT) {
        *length = 0;
        return;
    }

    q = &queues[descriptor];

    if (ss_simulink_queue_pop(descriptor, q->last, SS_SIMULINK_QUEUE_MAX_LEN, &len)) {
        q->last_len = len;
    }

    copy = (q->last_len > max_len) ? max_len : q->last_len;

    memcpy(data, q->last, copy);
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

#endif // COMPILE_SS_SIMULINK_QUEUE
