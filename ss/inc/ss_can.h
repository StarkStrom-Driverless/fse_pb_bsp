/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#ifndef _SS_CAN_H_
#define _SS_CAN_H_

#include <inttypes.h>
#include <stddef.h>
#include <FreeRTOS.h>
#include <queue.h>
#include "ss_clock.h"
#include "ss_rtos.h"

#define FIFO_SIZE 10
#define MAX_CAN_MSGS 10
#define SS_FILTER_BANKS 14
#define SS_FILTER_IDS (SS_FILTER_BANKS * 4)

struct SS_CAN_FRAME {
	uint32_t std_id;
	uint32_t ext_id;
	uint8_t ide;
	uint8_t rtr;
	uint8_t dlc;
	uint8_t data[8];
	uint8_t fmi;
};

struct SS_CAN_ID {
	uint32_t id;
	uint8_t is_ide;
	int8_t group_number;
};

struct SS_CAN_ID_FILTERS {
    struct SS_CAN_ID ids[SS_FILTER_IDS];
    uint8_t insert_pos;
    uint8_t free_id_group;
    uint8_t free_ide_group;
};

struct SS_CAN_TOD_MSG_STATUS {
	uint32_t std_id;
	uint16_t active_counter;
	uint16_t reset_value;
	uint8_t timeout_detected;
};

struct SS_TOD {
	struct SS_CAN_TOD_MSG_STATUS msgs[MAX_CAN_MSGS];
	uint8_t msg_count;
};

struct SS_CAN_MSG_QUEUE {
	QueueHandle_t queue;
	TaskHandle_t task_handle;
	uint32_t id;
};

struct SS_CAN_MSG_QUEUES {
	struct SS_CAN_MSG_QUEUE queues[MAX_CAN_MSGS];
	uint8_t insert_pos;
};

struct CAN_Channel {
	struct SS_CAN_MSG_QUEUES msg_queues;
	struct SS_TOD tod;
	struct SS_CAN_ID_FILTERS filters;
	struct SS_CAN_MSG_QUEUE std_msg_queue;
	bool enabled;
};


struct SS_CAN {
	struct CAN_Channel channel[2]; 
};

extern struct SS_CAN ss_can;



/***
 * 
 *  CAN USERSPACE FUNCTIONS
 * 
 */
SS_FEEDBACK ss_can_init(uint8_t can_interface_id, uint32_t baudrate);
SS_FEEDBACK ss_can_read(uint8_t can_interface_id, struct SS_CAN_FRAME* can_frame);
SS_FEEDBACK ss_can_send(uint8_t can_interface_id, struct SS_CAN_FRAME* can_frame);


/***
 * 
 *   CAN QUEUE FUNCTIONS
 * 
 */
SS_FEEDBACK ss_can_queue_std_init(uint8_t channel);
SS_FEEDBACK ss_can_queues_init(uint8_t channel);
char* u32_to_str(uint32_t value, char *buffer);
SS_FEEDBACK ss_can_queue_handle_add(uint8_t channel, 
                                    uint32_t id, 
                                    TaskFunction_t task_ptr, 
                                    void *const params, 
                                    uint8_t prio);
SS_FEEDBACK ss_can_queue_get(	uint8_t channel, 
								uint32_t id, 
								struct SS_CAN_MSG_QUEUE **queue);

SS_FEEDBACK ss_can_queue_read(struct SS_CAN_MSG_QUEUE *queue, struct SS_CAN_FRAME* frame);


/***
 * 
 *      CAN HELPER FUNCTIONS
 * 
 */
SS_FEEDBACK ss_can_enable_rcc(uint8_t can_interface_id);
SS_FEEDBACK ss_can_enable_gpios(uint8_t can_interface_id);
SS_FEEDBACK ss_can_nvic_init(uint8_t can_interface_id, uint8_t prio);
uint32_t ss_can_get_port_from_id(uint8_t can_interface_id);
uint32_t ss_can_get_fifo_from_channel(uint8_t channel);

/***
 * 
 *  CAN FRAME MANUPULATION FUNCTIONS
 * 
 */
void ss_can_frame_set_common(struct SS_CAN_FRAME *msg, uint32_t id, uint8_t dlc);
void ss_can_frame_set_signal(struct SS_CAN_FRAME *msg, uint8_t start_bit, uint8_t length, uint64_t value);
uint64_t ss_can_frame_get_signal(struct SS_CAN_FRAME* msg, uint8_t start_bit, uint8_t length);
void ss_can_frame_reset(struct SS_CAN_FRAME *msg);

/***
 * 
 *  CAN FRAME TIMEOUT DETECTION FUNCTIONS
 * 
 */
SS_FEEDBACK ss_can_tod_init(uint8_t channel);
SS_FEEDBACK ss_can_tod_add(uint8_t channel, uint32_t id, uint16_t reset_value);
SS_FEEDBACK ss_can_tod_check();
SS_FEEDBACK ss_can_tod_update(uint8_t channel, uint32_t id);


/***
 * 
 *  CAN FILTER ID HANDLING
 * 
 */
SS_FEEDBACK ss_can_filter_init(uint8_t channel);
SS_FEEDBACK ss_can_filter_add_msg_11(uint8_t channel, uint16_t id);
SS_FEEDBACK ss_can_filter_add_msg_28(uint8_t channel, uint32_t ide);
SS_FEEDBACK ss_can_filter_add_msg(uint8_t channel, uint32_t id);

#endif