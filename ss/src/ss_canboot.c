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

#if COMPILE_SS_CANBOOT
#include "ss_canboot.h"
#include "ss_can.h"
#include "ss_error.h"
#include <libopencm3/stm32/flash.h>

#include "ss_config.h"

#ifdef COMPILE_SS_PRINTF
#include "ss_printf.h"
#endif


struct SS_CANBOOT ss_canboot;

void canboot_task(void *args) {
    struct SS_CAN_FRAME frame;
    struct SS_CAN_MSG_QUEUE* queue;
    uint8_t received_update = 0;
    ss_can_queue_add(1, ss_canboot.can_id, &queue);

#ifdef COMPILE_SS_PRINTF
    ss_printf(4, "-> started <canboot_task:%d> \r\n", 0);
#endif

    for(;;) {
        if (ss_can_queue_read(queue, &frame)) {
            received_update = 1;
            flash_unlock();

            uint32_t baseaddr  = frame.data[4];
            baseaddr |= (frame.data[5] << 8);
            baseaddr |= (frame.data[6] << 16);
            baseaddr |= (frame.data[7] << 24);


            baseaddr += ss_canboot.flash_offset;

            uint32_t data  = frame.data[0];
            data |= (frame.data[1] << 8);
            data |= (frame.data[2] << 16);
            data |= (frame.data[3] << 24);

            flash_program_word(baseaddr, data);
            flash_wait_for_last_operation();

            flash_lock();

            ss_can_frame_reset(&frame);
            ss_can_frame_set_common(&frame, ss_canboot.can_id + 1, 4);
            
            ss_can_frame_set_signal(&frame, 0, 32, data);

            ss_can_send(1, &frame);
        }
        if (received_update == 0) {
            ss_rtos_delay_ms(200);
        } else {
            ss_rtos_delay_ms(2);
        }
        
    }
}

bool ss_canboot_init(uint32_t id) {
    uint32_t* start_address = (uint32_t*)CAN_BOOT_OFFSET;

    if (*start_address != 0xFFFFFFFF) {
        flash_unlock();

        for (uint8_t i = 0; i < 3; i++) {
            flash_erase_sector(i + 8, 2);
            flash_wait_for_last_operation();
        }

        flash_lock();
    }


    ss_canboot.can_id = id;
    ss_canboot.flash_offset = CAN_BOOT_OFFSET;

    if (ss_can.channel[0].enabled == false) {
        if (!ss_can_init(1, 1000000)) SS_ERROR(NULL);
    }

    /*
    if (!ss_can_filter_add_msg(1, id)) SS_ERROR(NULL);
    */

    if (!ss_rtos_task_add(canboot_task, NULL, 0, "canboot_task")) SS_ERROR(NULL);

    return true;
}
#endif // COMPILE_SS_CANBOOT
