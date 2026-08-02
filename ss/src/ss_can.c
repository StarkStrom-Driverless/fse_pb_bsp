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

#if COMPILE_SS_CAN
#include <libopencm3/stm32/can.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <inttypes.h>
#include <stddef.h>
#include "ss_gpio.h"
#include "ss_can.h"
#include "ss_feedback.h"
#include "ss_error.h"


#ifndef typeof
#define typeof __typeof__
#endif
#include "stb_ds.h"


#include "string.h"


#define SS_FEEDBACK_BASE SS_FEEDBACK_CAN_INIT_ERROR

struct SS_CAN ss_can;




/***
 * 
 *      ISR
 * 
 */
void can_isr(uint8_t channel) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t port = ss_can_get_port_from_id(channel);
    

    if (channel == 1) {
        if ((CAN_RF0R(port) & CAN_RF0R_FMP0_MASK) != 0) {
            struct SS_CAN_FRAME can_frame;
            struct SS_CAN_MSG_QUEUE* queue;
            
            ss_can_read(channel, &can_frame);
            
            if (ss_can_queue_get(channel, can_frame.std_id, &queue) == SS_FEEDBACK_OK) {
                xQueueSendFromISR(queue->queue, &can_frame, &xHigherPriorityTaskWoken);

                for (uint16_t i = 1; i <= queue->parallel_queue_id; i++) {
                    uint32_t id = SS_CAN_ID_PARALLEL(can_frame.std_id, i);

                    if (ss_can_queue_get(channel, id, &queue) == SS_FEEDBACK_OK) {
                        xQueueSendFromISR(queue->queue, &can_frame, &xHigherPriorityTaskWoken);
                    }
                }
            }
        }
        

        if (CAN_RF0R(port) & CAN_RF0R_FOVR0) {
            CAN_RF0R(port) &= ~CAN_RF0R_FOVR0;
        }
    } else if (channel == 2) {

        if ((CAN_RF1R(port) & CAN_RF1R_FMP1_MASK) != 0) {
            struct SS_CAN_FRAME can_frame;
            struct SS_CAN_MSG_QUEUE* queue;
            
            ss_can_read(channel, &can_frame);
            
            if (ss_can_queue_get(channel, can_frame.std_id, &queue) == SS_FEEDBACK_OK) {
                xQueueSendFromISR(queue->queue, &can_frame, &xHigherPriorityTaskWoken);
            }
        }

        if (CAN_RF1R(port) & CAN_RF1R_FOVR1) {
            CAN_RF1R(port) &= ~CAN_RF1R_FOVR1;
        }
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void can1_rx0_isr(void)
{
    can_isr(1);
}



void can2_rx1_isr(void)
{
    can_isr(2);
}



/***
 * 
 *      CAN HELPER FUNCTIONS
 * 
 */

bool ss_can_enable_rcc(uint8_t can_interface_id) {
    switch(can_interface_id) {
        case 1:
            rcc_periph_clock_enable(RCC_CAN1);
            break;

        case 2:
            rcc_periph_clock_enable(RCC_CAN2);
            break;

        default:
            SS_ERROR("unknown can_interface_id");
    }

    return true;
}

bool ss_can_enable_gpios(uint8_t can_interface_id) {
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
            SS_ERROR("unknown can_interface_id");
    }

    if (ss_io_init(tx, GPIO_MODE_AF) != SS_FEEDBACK_OK)      SS_ERROR("tx pin init failed");
    if (ss_io_init(rx, GPIO_MODE_AF) != SS_FEEDBACK_OK)      SS_ERROR("rx pin init failed");
    if (ss_io_init(stb, GPIO_MODE_OUTPUT) != SS_FEEDBACK_OK) SS_ERROR("stb pin init failed");

    ss_io_write(stb, SS_GPIO_ON);

    gpio_set_af(GPIO(PINBANK(tx)), GPIO_AF9, BIT(PINNO(tx)));
    gpio_set_af(GPIO(PINBANK(rx)), GPIO_AF9, BIT(PINNO(rx)));

    return true;
}

bool ss_can_nvic_init(uint8_t can_interface_id, uint8_t prio) {
    switch(can_interface_id) {
        case 1:
            nvic_enable_irq(NVIC_CAN1_RX0_IRQ);
            nvic_set_priority(NVIC_CAN1_RX0_IRQ, prio);
            break;

        case 2:
            nvic_enable_irq(NVIC_CAN2_RX1_IRQ);
            nvic_set_priority(NVIC_CAN2_RX1_IRQ, prio);
            break;

        default:
            SS_ERROR("unknown can_interface_id");
    }

    return true;
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

uint32_t ss_can_get_fifo_from_channel(uint8_t channel) {
    uint32_t fifo = 0;
    switch (channel)
    {
        case 0: fifo = CAN_FIFO0; break;
        case 1: fifo = CAN_FIFO1; break;
        default: break;
    }
    return fifo;
}

SS_FEEDBACK ss_can_get_id_type_from_id(uint32_t id) {
    SS_FEEDBACK rc = SS_FEEDBACK_CAN_MSG_IDE_INVALID;

    if (!(id & ~(0xFFFFFFF))) {
        if(id <= 0x7FF) {
            rc = SS_FEEDBACK_CAN_MSG_STD_ID;
        } else {
            rc = SS_FEEDBACK_CAN_MSG_IDE;
        }
    }

    return rc;
}

bool ss_can_enable_pending_interrupt(uint8_t channel, uint32_t can_port) {
    switch (channel)
    {
        case 0: can_enable_irq(can_port, CAN_IER_FMPIE0); break;
        case 1: can_enable_irq(can_port, CAN_IER_FMPIE1); break;

        default:
        break;
    }

    return true;
}


/***
 * 
 *  USERSPACE FUNCTIONS
 * 
 */

bool ss_can_init(uint8_t can_interface_id, uint32_t baudrate) {
    struct SS_CLOCK_CAN config;

    uint32_t can_port = ss_can_get_port_from_id(can_interface_id);
    if (can_port == 0) SS_ERROR("unknown can_interface_id");

    if (!ss_can_enable_gpios(can_interface_id)) SS_ERROR(NULL);

    if (!ss_can_enable_rcc(can_interface_id)) SS_ERROR(NULL);

    if (!ss_clock_can(&config, baudrate)) SS_ERROR(NULL);

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

    if (ret) SS_ERROR("libopencm3 can_init failed");

    if (!ss_can_nvic_init(can_interface_id, configMAX_SYSCALL_INTERRUPT_PRIORITY)) SS_ERROR(NULL);

    if (!ss_can_enable_pending_interrupt(can_interface_id - 1, can_port)) SS_ERROR(NULL);

    if (!ss_can_filter_init(can_interface_id - 1)) SS_ERROR(NULL);


    ss_can.channel[can_interface_id - 1].enabled = true;

    if (!ss_can_tod_init(can_interface_id - 1)) SS_ERROR(NULL);

    return true;
}

SS_FEEDBACK ss_can_read(uint8_t can_interface_id, struct SS_CAN_FRAME* can_frame) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    uint32_t can_port = ss_can_get_port_from_id(can_interface_id);
    uint8_t fifo = (can_interface_id == 2) ? 1 : 0;

    can_receive(    can_port,
                    fifo,
                    false,
                    &can_frame->std_id,
                    (bool*)&can_frame->ide,
                    (bool*)&can_frame->rtr,
                    &can_frame->fmi,
                    &can_frame->dlc,
                    can_frame->data,
                    0x0000);

    can_fifo_release(can_port, fifo);

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



SS_FEEDBACK ss_can_queue_get(uint8_t channel, uint32_t id, struct SS_CAN_MSG_QUEUE **queue) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    volatile uint32_t debug_id = id;

    channel--;

    struct SS_CAN_MSG_QUEUE_MAP* tmp = hmgetp_null(ss_can.channel[channel].msg_queues.map, id);
    if (tmp == NULL) {
        rc = SS_FEEDBACK_ERROR;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    *queue = &tmp->value;

    return rc;    
}

bool ss_can_queue_add(uint8_t channel, uint32_t id, struct SS_CAN_MSG_QUEUE **queue) {
    static bool init = false;
    struct SS_CAN_MSG_QUEUE* msg_queue;
    int16_t parallel_queue_id = 0;

    QueueHandle_t tmp = xQueueCreate(3, sizeof(struct SS_CAN_FRAME));
    if (tmp == NULL) {
        SS_ERROR("can queue create failed");
    }


    taskENTER_CRITICAL();
    channel--;


    if (init == false) {
        ss_can.channel[channel].msg_queues.map = NULL;
        init = true;
    } else {
        SS_FEEDBACK rc = ss_can_queue_get(channel + 1, id, &msg_queue);
        if (rc == SS_FEEDBACK_OK && msg_queue->parallel_queue_id < 15) {
            msg_queue->parallel_queue_id++;
            parallel_queue_id = msg_queue->parallel_queue_id;
            id = (parallel_queue_id << 28) | id;
        }
    }

    struct SS_CAN_MSG_QUEUE value = {
        .queue = tmp,
        .parallel_queue_id = parallel_queue_id
    };
    hmput(ss_can.channel[channel].msg_queues.map, id, value);

    ss_can_queue_get(channel + 1, id, queue);

    bool filter_ok = ss_can_filter_add_msg(channel + 1, SS_CAN_ID_RAW(id));

    taskEXIT_CRITICAL();

    if (!filter_ok) SS_ERROR(NULL);

    return true;
}

bool ss_can_queue_add_combined(uint8_t channel, uint32_t* ids, uint8_t len, struct SS_CAN_MSG_QUEUE **queue) {
    uint32_t first_id;

    channel--;

    if (len < 2) {
        SS_ERROR("combined queue needs at least 2 ids");
    }

    first_id = ids[0];
    if (!ss_can_queue_add(channel + 1, first_id, queue)) SS_ERROR(NULL);

    struct SS_CAN_MSG_QUEUE value = {
        .queue = hmgetp(ss_can.channel[channel].msg_queues.map, first_id)->value.queue,
        .parallel_queue_id = 0
    };

    for (int i = 1; i < len; i++) {
        hmput(ss_can.channel[channel].msg_queues.map, ids[i], value);
        ss_can_filter_add_msg(channel + 1, ids[i]);
    }

    return true;
}





SS_FEEDBACK ss_can_queue_read(struct SS_CAN_MSG_QUEUE *queue, struct SS_CAN_FRAME* frame) {
    SS_FEEDBACK rc = SS_FEEDBACK_CAN_NO_MSG_RECEIVED;

    if (xQueueReceive(queue->queue, frame, (TickType_t) 0 ) == pdPASS) {
        rc = SS_FEEDBACK_CAN_MSG_RECEIVED;
    }

    return rc;
}

SS_FEEDBACK ss_can_queue_has_msg(struct SS_CAN_MSG_QUEUE *queue) {
    SS_FEEDBACK rc = SS_FEEDBACK_CAN_NO_MSG_RECEIVED;

    UBaseType_t tmp = uxQueueMessagesWaiting(queue->queue);
    if (tmp > 0) {
        rc = SS_FEEDBACK_CAN_MSG_RECEIVED;
    }

    return rc;
}



/***
 * 
 *  CAN FILTER ID HANDLING
 * 
 */

bool ss_can_filter_init(uint8_t channel) {
    ss_can.channel[channel].filters.insert_pos = 0;
    ss_can.channel[channel].filters.free_id_group = 0;
    ss_can.channel[channel].filters.free_ide_group = SS_FILTER_BANKS - 1;

    for (uint8_t i = 0; i < SS_FILTER_IDS; i++) {
        struct SS_CAN_ID *id = &ss_can.channel[channel].filters.ids[i];
        id->id = 0;
        id->is_ide = 0;
        id->group_number = -1;
    }

    if (channel == 1) {
        CAN_FMR(CAN1) |= CAN_FMR_FINIT;
        CAN_FMR(CAN1) &= ~CAN_FMR_CAN2SB_MASK;
        CAN_FMR(CAN1) |= (SS_FILTER_BANKS << CAN_FMR_CAN2SB_SHIFT);
        CAN_FMR(CAN1) &= ~CAN_FMR_FINIT;
    }


    return true;
}

bool ss_can_filter_add_msg_11(uint8_t channel, uint16_t id) {
    channel--;

    uint8_t offset = (channel == 1) ? SS_FILTER_BANKS : 0;

    if (channel != 0 && channel != 1) {
        SS_ERROR("invalid can channel");
    }

    struct SS_CAN_ID_FILTERS *filters = &ss_can.channel[channel].filters;

    /* Kein freier Filter-Bank mehr: die aufsteigenden Standard-Baenke
     * (free_id_group) wuerden mit den absteigenden Extended-Baenken
     * (free_ide_group) kollidieren. Pruefung VOR dem HW-Schreibzugriff. */
    if (filters->free_id_group >= filters->free_ide_group) {
        SS_ERROR("can filter bank overrun");
    }

    uint16_t tmp[4] = {};

    filters->ids[filters->insert_pos].id = id;

    filters->ids[filters->insert_pos].group_number = filters->free_id_group;

    filters->insert_pos++;

    uint8_t cnt = 0;
    for (uint8_t i = 0; i < filters->insert_pos; i++) {
        if (filters->ids[i].is_ide) continue;

        if (filters->ids[i].group_number == filters->free_id_group) {
            tmp[cnt++] = (filters->ids[i].id << 5);
        }
    }


    
    can_filter_id_list_16bit_init(  filters->free_id_group + offset,
                                    tmp[0],
                                    tmp[1],
                                    tmp[2],
                                    tmp[3],
                                    channel,
                                    true);
    

    if (cnt == 4) {
        /* aktuelle Bank ist voll -> naechste Bank fuer den naechsten
         * Aufruf. Ob die noch frei ist, prueft der Check am Anfang. */
        filters->free_id_group++;
    }



    return true;
}

bool ss_can_filter_add_msg_28(uint8_t channel, uint32_t ide) {
    uint8_t offset = (channel == 1) ? SS_FILTER_BANKS : 0;

    channel--;


    if (channel != 0 && channel != 1) {
        SS_ERROR("invalid can channel");
    }

    struct SS_CAN_ID_FILTERS *filters = &ss_can.channel[channel].filters;
    uint32_t tmp[2] = {};

    filters->ids[filters->insert_pos].id = ide;

    filters->ids[filters->insert_pos].is_ide = 1;
    
    filters->ids[filters->insert_pos].group_number = filters->free_ide_group;

    filters->insert_pos++;


    uint8_t cnt = 0;
    for (uint8_t i = 0; i < filters->insert_pos; i++) {
        if (!filters->ids[i].is_ide) continue;

        if (filters->ids[i].group_number == filters->free_ide_group) {
            tmp[cnt++] = (filters->ids[i].id << 3) | CAN_TIxR_IDE;
        }
    }


    can_filter_id_list_32bit_init(  filters->free_ide_group,
                                    tmp[0],
                                    tmp[1],
                                    channel,
                                    true);


    if (cnt == 2) {
        filters->free_ide_group--;
        if ((filters->free_ide_group == filters->free_id_group)) {
            SS_ERROR("can id/ide filter bank collision");
        }
    }


    return true;
}

bool ss_can_filter_add_msg(uint8_t channel, uint32_t id) {
    SS_FEEDBACK id_type = ss_can_get_id_type_from_id(id);

    if (id_type == SS_FEEDBACK_CAN_MSG_IDE_INVALID) {
        SS_ERROR("can id fits neither 11 nor 29 bit format");
    }

    if (id_type == SS_FEEDBACK_CAN_MSG_IDE) {
        return ss_can_filter_add_msg_28(channel, id);
    }

    return ss_can_filter_add_msg_11(channel, id);
}



/***
 * 
 *  CAN FRAME TIMEOUT DETECTION FUNCTIONS
 * 
 */

bool ss_can_tod_init(uint8_t channel) {
    ss_can.channel[channel].tod.msg_count = 0;

    return true;
}


SS_FEEDBACK ss_can_tod_add(uint8_t channel, uint32_t id, uint16_t reset_value) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    if (channel == 1 || channel == 2) {
        channel--;
    } else {
        rc = SS_FEEDBACK_ERROR;
        return rc;
    }

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

    if (channel == 1 || channel == 2) {
        channel--;
    } else {
        rc = SS_FEEDBACK_ERROR;
        return rc;
    }

    struct SS_TOD* tod = &ss_can.channel[channel].tod;

    for (uint8_t i = 0; i < tod->msg_count; i++) {
        if (tod->msgs[i].std_id == id) {
            tod->msgs[i].active_counter = tod->msgs[i].reset_value;
            rc = SS_FEEDBACK_OK;
        }
    }

    return rc;
}

SS_FEEDBACK ss_can_tod_get(uint8_t channel, struct SS_TOD** tod_field) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    if (channel != 1 && channel != 2) {
        rc = SS_FEEDBACK_ERROR;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    channel--;

    *tod_field = &ss_can.channel[channel].tod;

    return rc;
}

SS_FEEDBACK ss_can_tod_check_field(struct SS_TOD* tod_field, uint8_t cnt, uint32_t* id, bool* tod_detected) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    if (tod_field == NULL) {
        rc = SS_FEEDBACK_ERROR;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    if (cnt > tod_field->msg_count) {
        rc = SS_FEEDBACK_ERROR;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    if(id != NULL) {
        *id = tod_field->msgs[cnt].std_id;
    }
    
    if(tod_detected != NULL) {
        *tod_detected = (tod_field->msgs[cnt].timeout_detected) ? true : false;
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
#endif // COMPILE_SS_CAN
