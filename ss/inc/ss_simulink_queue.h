/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_SIMULINK_QUEUE

#ifndef __SS_SIMULINK_QUEUE_H__
#define __SS_SIMULINK_QUEUE_H__

#include <inttypes.h>
#include <stdbool.h>

#ifndef SS_SIMULINK_QUEUE_COUNT
#define SS_SIMULINK_QUEUE_COUNT 8
#endif

#ifndef SS_SIMULINK_QUEUE_MAX_LEN
#define SS_SIMULINK_QUEUE_MAX_LEN 64
#endif

#ifndef SS_SIMULINK_QUEUE_DEPTH
#define SS_SIMULINK_QUEUE_DEPTH 8
#endif

bool ss_simulink_queue_init(uint8_t descriptor, uint16_t max_len, uint8_t depth);

bool ss_simulink_queue_push(uint8_t descriptor, const uint8_t *data, uint32_t length);

bool ss_simulink_queue_pop(uint8_t descriptor, uint8_t *data, uint32_t max_len,
                           uint32_t *length);

void ss_simulink_queue_pop_latest(uint8_t descriptor, uint8_t *data, uint32_t max_len,
                                  uint32_t *length);

uint8_t ss_simulink_queue_overrun(uint8_t descriptor);

#endif // __SS_SIMULINK_QUEUE_H__

#endif // COMPILE_SS_SIMULINK_QUEUE
