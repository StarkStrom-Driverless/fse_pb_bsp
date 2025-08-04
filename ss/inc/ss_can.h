#ifndef _SS_CAN_H_
#define _SS_CAN_H_

#include <inttypes.h>
#include <stddef.h>


#include <FreeRTOS.h>
#include <queue.h>

#include "ss_clock.h"

#define FIFO_SIZE 10
#define MAX_TIMEOUT_DETECTION 10

struct can_tx_msg {
	uint32_t std_id;
	uint32_t ext_id;
	uint8_t ide;
	uint8_t rtr;
	uint8_t dlc;
	uint8_t data[8];
};

struct can_rx_msg {
	uint32_t std_id;
	uint32_t ext_id;
	uint8_t ide;
	uint8_t rtr;
	uint8_t dlc;
	uint8_t data[8];
	uint8_t fmi;
};

struct MsgStatus {
	uint32_t std_id;
	uint16_t active_counter;
	uint16_t reset_value;
	uint8_t timeout_detected;
};

struct TimeOutDetection {
	struct MsgStatus msgs[MAX_TIMEOUT_DETECTION];
	uint8_t msg_count;
};


struct Fifo {
    struct can_rx_msg can_frames[FIFO_SIZE];
    int front;
    int rear;
};

struct CAN_Mgs {
	uint32_t id;
	QueueHandle_t queue;
	TaskHandle_t task_handle;
};

struct CAN_Channel {
	uint8_t insert_pos;
	struct CAN_Mgs msg[10];
};

struct SS_CAN {
	struct CAN_Channel channel[2]; 
};

extern struct SS_CAN ss_can;

extern QueueHandle_t can_queues[2];

int8_t ss_enable_can_rcc(uint8_t can_interface_id);

int8_t ss_enable_can_gpios(uint8_t can_interface_id);

int8_t ss_init_can_nvic(uint8_t can_interface_id, uint8_t prio);

int8_t ss_can_get_bit_timings(uint32_t baudrate, uint32_t* sjw, uint32_t* tseg1, uint32_t* tseg2, uint32_t* prescaler);

uint32_t ss_get_can_port_from_id(uint8_t can_interface_id);

struct SS_CAN* ss_can_init(uint8_t can_interface_id, uint32_t baudrate, struct SS_CLOCK* ss_clock);

struct CAN_Mgs* ss_can_add_message_queue(uint8_t can_interface_id,  uint32_t id);

struct CAN_Mgs* ss_can_get_message_handle_from_id(uint8_t can_interface_id,  uint32_t id);

int8_t ss_can_read(uint8_t can_interface_id, struct can_rx_msg* can_frame);

int8_t ss_can_send(uint8_t can_interface_id, struct can_tx_msg* can_frame);

int8_t ss_can_add_messages(uint32_t* ids, uint8_t len);

void usb_lp_can_rx0_isr(void);

void ss_can_reset_frame(struct can_tx_msg* msg);

void ss_can_set_common_to_frame(struct can_tx_msg* msg, uint32_t id, uint8_t dlc);

void ss_can_set_signal_to_frame(struct can_tx_msg* msg, uint8_t start_bit, uint8_t length, uint64_t value);

uint64_t ss_can_get_signal_from_frame(struct can_rx_msg* msg, uint8_t start_bit, uint8_t length);

uint8_t ss_can_frame_received(struct can_rx_msg* msg, QueueHandle_t queue);


void ss_can_init_timeout_detection(struct TimeOutDetection* tod);
uint8_t ss_can_add_timeout(struct TimeOutDetection* tod, uint32_t id, uint16_t reset_value);
uint8_t ss_can_check_timeout_detection(struct TimeOutDetection* tod);
void ss_can_update_timeout_detection(struct TimeOutDetection* tod, uint32_t id);


#endif