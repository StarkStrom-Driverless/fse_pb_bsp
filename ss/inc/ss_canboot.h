/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_CANBOOT

#ifndef _SS_CAN_BOOT_H_
#define _SS_CAN_BOOT_H_

#define CAN_BOOT_OFFSET 0x08080000

#include "ss_feedback.h"

struct SS_CANBOOT {
    uint32_t can_id;
    uint32_t flash_offset;
};

extern struct SS_CANBOOT ss_canboot;

#ifdef USE_PRIVATE
static void canboot_task(void* args);
#endif

SS_FEEDBACK ss_canboot_init(uint32_t id);

#endif // _SS_CAN_BOOT_H_

#endif // COMPILE_SS_CANBOOT
