#include <string.h>
#include "ss_can.h"

struct SS_CAN ss_can;

#define SIM_QUEUE_SLOTS 32

static struct SS_CAN_MSG_QUEUE sim_queues[SIM_QUEUE_SLOTS];
static uint32_t sim_queue_ids[SIM_QUEUE_SLOTS];
static uint8_t sim_queue_count;

bool ss_can_init(uint8_t can_interface_id, uint32_t baudrate) {
    return true;
}

bool ss_can_send(uint8_t can_interface_id, struct SS_CAN_FRAME* can_frame) {
    return true;
}

bool ss_can_read(uint8_t can_interface_id, struct SS_CAN_FRAME* can_frame) {
    return false;
}

bool ss_can_queue_add(uint8_t channel, uint32_t id, struct SS_CAN_MSG_QUEUE **queue) {
    if (sim_queue_count >= SIM_QUEUE_SLOTS) {
        return false;
    }

    sim_queue_ids[sim_queue_count] = id;
    sim_queues[sim_queue_count].queue = 0;
    sim_queues[sim_queue_count].parallel_queue_id = 0;

    *queue = &sim_queues[sim_queue_count];
    sim_queue_count++;

    return true;
}

uint8_t ss_can_queue_read(struct SS_CAN_MSG_QUEUE *queue, struct SS_CAN_FRAME* frame) {
    return 0;
}

void ss_can_frame_set_common(struct SS_CAN_FRAME *msg, uint32_t id, uint8_t dlc) {
    msg->std_id = id;
    msg->dlc = dlc;
}

void ss_can_frame_reset(struct SS_CAN_FRAME *msg) {
    memset(msg, 0, sizeof(*msg));
}

void ss_can_frame_set_signal(struct SS_CAN_FRAME *msg, uint8_t start_bit, uint8_t length, uint64_t value) {
    uint64_t data = 0;
    uint64_t mask;
    int i;

    for (i = 0; i < 8; ++i) {
        data |= ((uint64_t) msg->data[i]) << (i * 8);
    }

    mask = (length == 64) ? ~0ULL : ((1ULL << length) - 1);

    data &= ~(mask << start_bit);
    data |= (value & mask) << start_bit;

    for (i = 0; i < 8; ++i) {
        msg->data[i] = (uint8_t) (data >> (i * 8));
    }
}

uint64_t ss_can_frame_get_signal(struct SS_CAN_FRAME* msg, uint8_t start_bit, uint8_t length) {
    uint64_t data = 0;
    uint64_t mask;
    int i;

    for (i = 0; i < 8; ++i) {
        data |= ((uint64_t) msg->data[i]) << (i * 8);
    }

    mask = (length == 64) ? ~0ULL : ((1ULL << length) - 1);

    return (data >> start_bit) & mask;
}
