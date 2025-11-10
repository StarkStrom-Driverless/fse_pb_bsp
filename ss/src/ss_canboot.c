#include "ss_canboot.h"
#include "ss_can.h"
#include <libopencm3/stm32/flash.h>

#define SS_FEEDBACK_BASE SS_FEEDBACK_CANBOOT_INIT_ERROR

struct SS_CANBOOT ss_canboot;

void canboot_task(void *args) {
    struct SS_CAN_FRAME frame;
    struct SS_CAN_MSG_QUEUE* queue;
    uint8_t received_update = 0;
    ss_can_queue_get(1, ss_canboot.can_id, &queue);
    for(;;) {
        if (ss_can_queue_read(queue, &frame) == SS_FEEDBACK_CAN_MSG_RECEIVED) {
            received_update = 1;
            flash_unlock();

            //uint32_t baseaddr = ss_can_frame_get_signal(&frame, 32, 20);

            uint32_t baseaddr  = frame.data[4];
            baseaddr |= (frame.data[5] << 8);
            baseaddr |= (frame.data[6] << 16);
            baseaddr |= (frame.data[7] << 24);

            /*
            if (baseaddr == 0x00) {
                flash_unlock();

                for (uint8_t i = 0; i < 3; i++) {
                    flash_erase_sector(i + 8, 2);
                    flash_wait_for_last_operation();
                }

                flash_lock();
            }
                */

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

SS_FEEDBACK ss_canboot_init(uint32_t id, uint32_t offset) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    
    flash_unlock();

    for (uint8_t i = 0; i < 3; i++) {
        flash_erase_sector(i + 8, 2);
        flash_wait_for_last_operation();
    }

    flash_lock();
    

    ss_canboot.can_id = id;
    ss_canboot.flash_offset = offset;

    if (ss_can.channel[0].enabled == false) {
        rc = ss_can_init(1, 1000000);
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_can_filter_add_msg(1, id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_rtos_task_add(canboot_task, NULL, 3, "canboot_task");

    return rc;
}