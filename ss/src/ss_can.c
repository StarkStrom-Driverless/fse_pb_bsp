#include <libopencm3/stm32/can.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <inttypes.h>
#include <stddef.h>
#include "ss_gpio.h"
#include "ss_can.h"
#include "ss_feedback.h"

#include "string.h"


#define SS_FEEDBACK_BASE SS_FEEDBACK_CAN_INIT_ERROR

struct SS_CAN ss_can;




/***
 * 
 *      ISR
 * 
 */
void can1_rx0_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ((CAN_RF0R(CAN1) & CAN_RF0R_FMP0_MASK) != 0) {
        struct SS_CAN_FRAME can_frame;
        struct SS_CAN_MSG_QUEUE* queue;

        ss_can_read(1, &can_frame);

        if (ss_can_queue_get(1, can_frame.std_id, queue) == SS_FEEDBACK_OK) {
            xQueueSendFromISR(queue->queue, &can_frame, &xHigherPriorityTaskWoken);
        }
    }

    // Überlauf behandeln!
    if (CAN_RF0R(CAN1) & CAN_RF0R_FOVR0) {
        CAN_RF0R(CAN1) &= ~CAN_RF0R_FOVR0;
    }

    // Context-Switch bei Bedarf
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


/***
 * 
 *      CAN HELPER FUNCTIONS
 * 
 */

SS_FEEDBACK ss_can_enable_rcc(uint8_t can_interface_id) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    switch(can_interface_id) {
        case 1:
            rcc_periph_clock_enable(RCC_CAN1);
            break;
        
        case 2:
            rcc_periph_clock_enable(RCC_CAN2);
            break;

        default:
            rc = SS_FEEDBACK_CAN_PIN_RCC_ERROR;
            break;
    }

    return rc;
}

SS_FEEDBACK ss_can_enable_gpios(uint8_t can_interface_id) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    uint16_t tx = 0;
    uint16_t rx = 0;
    uint16_t stb = 0;

    switch(can_interface_id) {
        case 1:
            tx = PIN('B', 9);
            rx = PIN('B', 8);
            stb = PIN('B', 7);
            break;
        
        case 2:
            tx = PIN('B', 6);
            rx = PIN('B', 5);
            stb = PIN('B', 4);
            break;

        default:
            rc = SS_FEEDBACK_CAN_PIN_CONFIG_ERROR;
            break;
    }

    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_io_init(tx, GPIO_MODE_AF);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_io_init(rx, GPIO_MODE_AF);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_io_init(stb, GPIO_MODE_OUTPUT);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    ss_io_write(stb, SS_GPIO_ON);

    gpio_set_af(GPIO(PINBANK(tx)), GPIO_AF9, BIT(PINNO(tx)));
    gpio_set_af(GPIO(PINBANK(rx)), GPIO_AF9, BIT(PINNO(rx)));

    return rc;
}

SS_FEEDBACK ss_can_nvic_init(uint8_t can_interface_id, uint8_t prio) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    switch(can_interface_id) {
        case 1:
            nvic_enable_irq(NVIC_CAN1_RX0_IRQ);
            nvic_set_priority(NVIC_CAN1_RX0_IRQ, prio);
            break;
        
        case 2:
            nvic_enable_irq(NVIC_CAN2_RX0_IRQ);
            nvic_set_priority(NVIC_CAN2_RX0_IRQ, prio);
            break;

        default:
            rc = SS_FEEDBACK_CAN_PERIPH_ERROR;
            break;
    }

    return rc;
}



uint32_t ss_can_get_port_from_id(uint8_t can_interface_id) {
    uint32_t can_port = 0;
    switch (can_interface_id)
    {
        case 1:
            can_port = CAN1;
            break;

        case 2:
            can_port = CAN2;
            break;

        default:
            break;
    }

    return can_port;
}


/***
 * 
 *  USERSPACE FUNCTIONS
 * 
 */

SS_FEEDBACK ss_can_init(uint8_t can_interface_id, uint32_t baudrate) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    struct SS_CLOCK_CAN config; 

    uint32_t can_port = ss_can_get_port_from_id(can_interface_id);
    SS_HANDLE_NULL_WITH_EXIT(can_port);

    rc = ss_can_enable_gpios(can_interface_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_can_enable_rcc(can_interface_id);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_clock_can(&config, baudrate);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    can_reset(can_port);

    uint8_t ret = can_init( can_port,
                            false,
                            true,
                            false,
                            false,
                            false,
                            false,
                            config.sjw,
                            config.tseg1,
                            config.tseg2,
                            config.prescaler,
                            false,
                            false);
    if (ret) return SS_FEEDBACK_CAN_INIT_ERROR;


    rc = ss_can_nvic_init(can_interface_id, 1);
    SS_HANDLE_ERROR_WITH_EXIT(rc);
                                    
    can_enable_irq(can_port, CAN_IER_FMPIE0);

    rc = ss_can_filter_init(can_interface_id - 1);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_can_queue_std_init(can_interface_id - 1);
    SS_HANDLE_ERROR_WITH_EXIT(rc);


    return rc;
}

SS_FEEDBACK ss_can_read(uint8_t can_interface_id, struct SS_CAN_FRAME* can_frame) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    uint32_t can_port = ss_can_get_port_from_id(can_interface_id);

    can_receive(    can_port,
                    0,
                    false,
                    &can_frame->std_id,
                    &can_frame->ide,
                    &can_frame->rtr,
                    &can_frame->fmi,
                    &can_frame->dlc,
                    can_frame->data,
                    0x0000);

    can_fifo_release(can_port, 0);

    return rc;
}

SS_FEEDBACK ss_can_send(uint8_t can_interface_id, struct SS_CAN_FRAME* can_frame) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    uint32_t can_port = ss_can_get_port_from_id(can_interface_id);
    
    int8_t ret =  can_transmit(   can_port,
                    can_frame->std_id,
                    can_frame->ide,
                    can_frame->rtr,
                    can_frame->dlc,
                    can_frame->data);

    return rc;
}




/***
 * 
 *   CAN QUEUE FUNCTIONS
 * 
 */

SS_FEEDBACK ss_can_queue_std_init(uint8_t channel) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    QueueHandle_t queue = xQueueCreate(10, sizeof(struct SS_CAN_FRAME));
    if (queue == NULL) {
        rc = SS_FEEDBACK_CAN_QUEUE_CREATE_ERROR;
    }
    ss_can.channel[channel].std_msg_queue.queue = queue;

    return rc;
}

SS_FEEDBACK ss_can_queues_init(uint8_t channel) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    ss_can.channel[channel].msg_queues.insert_pos = 0;

    return rc;
}

char* u32_to_str(uint32_t value, char *buffer) {
    char *p = buffer;
    // Sonderfall 0
    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return buffer;
    }

    // Ziffern rückwärts ins Bufferende schreiben
    char *start = p;
    while (value > 0) {
        *p++ = '0' + (value % 10);
        value /= 10;
    }

    // String umdrehen
    *p = '\0';
    for (char *q = start, *r = p - 1; q < r; q++, r--) {
        char tmp = *q;
        *q = *r;
        *r = tmp;
    }

    return buffer;
}

SS_FEEDBACK ss_can_queue_handle_add(uint8_t channel, 
                                    uint32_t id, 
                                    TaskFunction_t task_ptr, 
                                    void *const params, 
                                    uint8_t prio) 
{
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    struct SS_CAN_MSG_QUEUES* queue = &ss_can.channel[channel].msg_queues;
    QueueHandle_t tmp = NULL;
    char name[20];
    char id_str[8];

    if (queue->insert_pos >= MAX_CAN_MSGS) {
        rc = SS_FEEDBACK_CAN_QUEUE_OVERRUN;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    queue->queues[queue->insert_pos].id = id;

    tmp = xQueueCreate(10, sizeof(struct SS_CAN_FRAME));
    if (tmp == NULL) {
        rc = SS_FEEDBACK_CAN_QUEUE_CREATE_ERROR;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    queue->queues[queue->insert_pos].queue = tmp;

    strcpy(name, "queue_");
    u32_to_str(id, id_str);
    strcat(name, id_str);


    rc = ss_rtos_task_add(  task_ptr,
                            params,
                            prio,
                            name);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    queue->insert_pos++;

    return rc;
}


SS_FEEDBACK ss_can_queue_get(uint8_t channel, uint32_t id, struct SS_CAN_MSG_QUEUE **queue) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;


    for (uint8_t i = 0; i < ss_can.channel[channel].msg_queues.insert_pos; i++) {
        if (ss_can.channel[channel].msg_queues.queues[i].id == id) {
            queue = &ss_can.channel[channel].msg_queues.queues[i];
            return rc;
        }
    }

    *queue = &ss_can.channel[channel].std_msg_queue;

    return rc;    
}



SS_FEEDBACK ss_can_queue_read(struct SS_CAN_MSG_QUEUE *queue, struct SS_CAN_FRAME* frame) {
    SS_FEEDBACK rc = SS_FEEDBACK_CAN_NO_MSG_RECEIVED;

    if (xQueueReceive(queue->queue, frame, (TickType_t) 0 ) == pdPASS) {
        rc = SS_FEEDBACK_CAN_MSG_RECEIVED;
    }

    return rc;
}



/***
 * 
 *  CAN FILTER ID HANDLING
 * 
 */

SS_FEEDBACK ss_can_filter_init(uint8_t channel) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;




    ss_can.channel[channel].filters.insert_pos = 0;
    ss_can.channel[channel].filters.free_id_group = 0;
    ss_can.channel[channel].filters.free_ide_group = 0;

    for (uint8_t i = 0; i < SS_FILTER_IDS; i++) {
        struct SS_CAN_ID *id = &ss_can.channel[channel].filters.ids[i];
        id->id = 0;
        id->is_ide = 0;
        id->group_number = -1;
    }

    return rc;
}

SS_FEEDBACK ss_can_filter_add_msg_11(uint8_t channel, uint16_t id) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    channel--;

    if (channel != 0 && channel != 1) {
        rc = SS_FEEDBACK_CAN_PERIPH_ERROR;
        SS_HANDLE_ERROR_WITH_EXIT(rc);
    }

    struct SS_CAN_ID_FILTERS *filters = &ss_can.channel[channel].filters;
    uint16_t tmp[4] = {};

    filters->ids[filters->insert_pos].id = id;

    filters->ids[filters->insert_pos].group_number = filters->free_id_group;

    filters->insert_pos++;

    uint8_t cnt = 0;
    for (uint8_t i = 0; i < filters->insert_pos; i++) {
        if (filters->ids[i].is_ide) continue;

        if (filters->ids[i].group_number == filters->free_id_group) {
            tmp[cnt++] = filters->ids[i].id;
        }
    }

    can_filter_id_list_16bit_init(  filters->free_id_group,
                                    tmp[0],
                                    tmp[1],
                                    tmp[2],
                                    tmp[3],
                                    channel,
                                    true);

    if (cnt == 4) {
        filters->free_id_group++;
        if ((filters->free_ide_group + filters->free_id_group) >= SS_FILTER_BANKS) {
            rc = SS_FEEDBACK_CAN_FILTER_OVERRUN;
            SS_HANDLE_ERROR_WITH_EXIT(rc);
        }
    }

    

    return rc;
}

SS_FEEDBACK ss_can_filter_add_msg_28(uint8_t channel, uint32_t ide) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    channel--;

    if (channel != 0 && channel != 1) {
        rc = SS_FEEDBACK_CAN_PERIPH_ERROR;
        SS_HANDLE_ERROR_WITH_EXIT(rc);
    }

    struct SS_CAN_ID_FILTERS *filters = &ss_can.channel[channel].filters;
    uint16_t tmp[2] = {};

    filters->ids[filters->insert_pos].id = ide;

    filters->ids[filters->insert_pos].is_ide = 1;
    
    filters->ids[filters->insert_pos].group_number = filters->free_ide_group;

    filters->insert_pos++;


    uint8_t cnt = 0;
    for (uint8_t i = 0; i < filters->insert_pos; i++) {
        if (!filters->ids[i].is_ide) continue;

        if (filters->ids[i].group_number == filters->free_ide_group) {
            tmp[cnt++] = filters->ids[i].id;
        }
    }


    can_filter_id_list_32bit_init(  filters->free_ide_group,
                                    tmp[0],
                                    tmp[1],
                                    channel,
                                    1);


    if (cnt == 2) {
        filters->free_ide_group++;
        if ((filters->free_ide_group + filters->free_id_group) >= SS_FILTER_BANKS) {
            return 2;
        }
    }


    return rc;
}

SS_FEEDBACK ss_can_filter_add_msg(uint8_t channel, uint32_t id) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    if (id & ~((uint32_t)(0b11111111111))) {
        rc = ss_can_filter_add_msg_28(channel, id);
    } else {
        rc = ss_can_filter_add_msg_11(channel, id);
    }

    return rc;
}



/***
 * 
 *  CAN FRAME TIMEOUT DETECTION FUNCTIONS
 * 
 */

SS_FEEDBACK ss_can_tod_init(uint8_t channel) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    ss_can.channel[channel].tod.msg_count = 0;

    return rc;
}


SS_FEEDBACK ss_can_tod_add(uint8_t channel, uint32_t id, uint16_t reset_value) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    struct SS_TOD* tod = &ss_can.channel[channel].tod;

    tod->msgs[tod->msg_count].std_id = id;
    tod->msgs[tod->msg_count].active_counter = reset_value;
    tod->msgs[tod->msg_count].reset_value = reset_value;
    tod->msgs[tod->msg_count].timeout_detected = 0;

    if (tod->msg_count > MAX_CAN_MSGS) {
        rc = SS_FEEDBACK_CAN_TOD_OVERRUN;
    } else {
        tod->msg_count++;
    }
    
    return rc;
}

SS_FEEDBACK ss_can_tod_check() {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    for (uint8_t j = 0; j < 2; j++) {
        struct SS_TOD* tod = &ss_can.channel[j].tod;

        for (uint8_t i = 0; i < tod->msg_count; i++) {
            if (tod->msgs[i].active_counter == 0) {
                rc = SS_FEEDBACK_CAN_TOD_HAPPEND;
                tod->msgs[i].timeout_detected = 1;
            } else {
                tod->msgs[i].timeout_detected = 0;
                tod->msgs[i].active_counter--;
            }
        }
    }

    return rc;
}

SS_FEEDBACK ss_can_tod_update(uint8_t channel, uint32_t id) {
    SS_FEEDBACK rc = SS_FEEDBACK_CAN_TOD_ID_NOT_FOUND;
    struct SS_TOD* tod = &ss_can.channel[channel].tod;

    for (uint8_t i = 0; i < tod->msg_count; i++) {
        if (tod->msgs[i].std_id == id) {
            tod->msgs[i].active_counter = tod->msgs[i].reset_value;
            rc = SS_FEEDBACK_OK;
        }
    }

    return rc;
}



/***
 * 
 *          CAN FRAME MANUPULATION FUNCTIONS
 * 
 */
void ss_can_frame_set_common(struct SS_CAN_FRAME *msg, uint32_t id, uint8_t dlc) {
    msg->std_id = id;
    msg->dlc = dlc;
    msg->rtr = 0;
    msg->ide = 0;
}

void ss_can_frame_set_signal(struct SS_CAN_FRAME *msg, uint8_t start_bit, uint8_t length, uint64_t value) {
    uint64_t data = 0;
    for (int i = 0; i < 8; ++i) {
        data |= ((uint64_t)msg->data[i]) << (i * 8);
    }

    uint64_t mask = (length == 64) ? ~0ULL : ((1ULL << length) - 1);
    value &= mask;
    mask <<= start_bit;
    data = (data & ~mask) | (value << start_bit);

    for (int i = 0; i < 8; ++i) {
        msg->data[i] = (data >> (i * 8)) & 0xFF;
    }
}

uint64_t ss_can_frame_get_signal(struct SS_CAN_FRAME* msg, uint8_t start_bit, uint8_t length) {
    uint64_t data = 0;

    for (int i = 0; i < 8; ++i) {
        data |= ((uint64_t)msg->data[i]) << (i * 8);
    }


    uint64_t mask = (length == 64) ? ~0ULL : ((1ULL << length) - 1);
    return (data >> start_bit) & mask;
}

void ss_can_frame_reset(struct SS_CAN_FRAME *msg) {
    msg->std_id = 0;
    msg->ext_id = 0;
    msg->ide = 0;
    msg->rtr = 0;
    msg->dlc = 0;
    msg->data[0] = 0;
    msg->data[1] = 0;
    msg->data[2] = 0;
    msg->data[3] = 0;
    msg->data[4] = 0;
    msg->data[5] = 0;
    msg->data[6] = 0;
    msg->data[7] = 0;
}