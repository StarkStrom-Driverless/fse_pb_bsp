/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#ifndef _SS_CAN_BOOT_H_
#define _SS_CAN_BOOT_H_


#include "ss_feedback.h"

struct SS_CANBOOT {
    uint32_t can_id;
    uint32_t flash_offset;
};

extern struct SS_CANBOOT ss_canboot;

static void canboot_task(void* args);

SS_FEEDBACK ss_canboot_init(uint32_t id, uint32_t offset);

#endif