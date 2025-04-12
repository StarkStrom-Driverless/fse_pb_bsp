#include <libopencm3/stm32/can.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <inttypes.h>
#include <stddef.h>
#include "ss_gpio.h"
#include "ss_can.h"

int8_t ss_enable_can_rcc(uint8_t can_interface_id) {
    int8_t status = 0;
    switch(can_interface_id) {
        case 1:
            rcc_periph_clock_enable(RCC_CAN1);
            break;
        
        case 2:
            rcc_periph_clock_enable(RCC_CAN2);
            break;

        default:
            status = -1;
            break;
    }

    return status;
}

int8_t ss_enable_can_gpios(uint8_t can_interface_id) {
    int8_t status = 0;

    uint16_t tx = 0;
    uint16_t rx = 0;
    uint16_t stb = 0;

    switch(can_interface_id) {
        case 1:
            tx = ss_io_init(PIN('B', 9), GPIO_MODE_AF);
            rx = ss_io_init(PIN('B', 8), GPIO_MODE_AF);
            stb = ss_io_init(PIN('B', 7), GPIO_MODE_OUTPUT);
            ss_io_write(stb, SS_GPIO_ON);
            break;
        
        case 2:
            tx = ss_io_init(PIN('B', 6), GPIO_MODE_AF);
            rx = ss_io_init(PIN('B', 5), GPIO_MODE_AF);
            stb = ss_io_init(PIN('B', 4), GPIO_MODE_OUTPUT);
            ss_io_write(stb, SS_GPIO_ON);
            break;

        default:
            status = -1;
            break;
    }

    if (status == 0) {
        gpio_set_af(GPIO(PINBANK(tx)), GPIO_AF9, BIT(PINNO(tx)));
        gpio_set_af(GPIO(PINBANK(rx)), GPIO_AF9, BIT(PINNO(rx)));
    }

    return status;
}

int8_t ss_init_can_nvic(uint8_t can_interface_id, uint8_t prio) {
    int8_t status = 0;

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
            status = -1;
            break;
    }

    return status;
}

int8_t ss_can_get_bit_timings(uint32_t baudrate, uint32_t* sjw, uint32_t* tseg1, uint32_t* tseg2, uint32_t* prescaler) {
    int8_t status = 0;
    
    switch(baudrate) {
        case 1000000: 
            *prescaler = 1;
            *tseg1 = CAN_BTR_TS1_12TQ;
            *tseg2 = CAN_BTR_TS2_3TQ;
            *sjw = CAN_BTR_SJW_1TQ;
            break;

        case 500000: 
            *prescaler = 6;
            *tseg1 = 12;
            *tseg2 = 3;
            *sjw = 1;
            break;

        case 250000: 
            *prescaler = 12;
            *tseg1 = 12;
            *tseg2 = 3;
            *sjw = 1;
            break;

        default: status = -1; break;
    }

    return status;
}

uint32_t ss_get_can_port_from_id(uint8_t can_interface_id) {
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

int8_t ss_can_init(uint8_t can_interface_id, uint32_t baudrate) {
    int8_t status = 0;
    uint32_t prescaler = 0;
    uint32_t sjw = 0;
    uint32_t tseg1 = 0;
    uint32_t tseg2 = 0;

    uint32_t can_port = ss_get_can_port_from_id(can_interface_id);

    if (ss_enable_can_gpios(can_interface_id) == -1) status = -1;

    if (ss_enable_can_rcc(can_interface_id) == -1) status = -1;

    if (ss_can_get_bit_timings(baudrate, &sjw, &tseg1, &tseg2, &prescaler) == -1) status = -1;

    can_reset(can_port);

    uint8_t ret = can_init( can_port,
                            false,
                            true,
                            false,
                            false,
                            false,
                            false,
                            sjw,
                            tseg1,
                            tseg2,
                            prescaler,
                            false,
                            false);

    
    can_filter_id_mask_32bit_init(  0,
                                    0,
                                    0,
                                    0,
                                    true);
    



    if (ss_init_can_nvic(can_interface_id, 1) == -1) status = -1;    
                                    
    can_enable_irq(can_port, CAN_IER_FMPIE0);

    nvic_enable_irq(NVIC_CAN1_SCE_IRQ);
    can_enable_irq(CAN1, CAN_IER_ERRIE);
}

int8_t ss_can_add_messages(uint32_t* ids, uint8_t len) {
    int counter = 0;

    uint32_t std_ids[28];
    uint32_t std_ids_count = 0;
    uint32_t ext_ids[28];
    uint32_t ext_ids_count = 0;

    if (len > 28) return -1;

    
    for (int k = 0; k < len; k++) {
        if (ids[k] & ~(0x7FF)) {
            ext_ids[ext_ids_count++] = ids[k];
        } else {
            std_ids[std_ids_count++] = ids[k];
        }
    }

    
    int i = 0;
    while (i < std_ids_count) {
        uint32_t ida = std_ids[i++];
        uint32_t idb = (i < std_ids_count) ? std_ids[i++] : 0;

        can_filter_id_list_32bit_init(counter++, ida << 21, idb << 21, 0, true);
    }

    
    i = 0;
    while (i < ext_ids_count) {
        uint32_t ida = ext_ids[i++];
        uint32_t idb = (i < ext_ids_count) ? ext_ids[i++] : 0;

        can_filter_id_list_32bit_init(counter++, (ida << 3) | CAN_TIxR_IDE, (idb << 3) | CAN_TIxR_IDE, 1, true);
    }

    return 0;
}

int8_t ss_can_read(uint8_t can_interface_id, struct can_rx_msg* can_frame) {
    uint32_t can_port = ss_get_can_port_from_id(can_interface_id);

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

    return 0;
}

int8_t ss_can_send(uint8_t can_interface_id, struct can_tx_msg* can_frame) {
    uint32_t can_port = ss_get_can_port_from_id(can_interface_id);
    
    can_transmit(   can_port,
                    can_frame->std_id,
                    can_frame->ide,
                    can_frame->rtr,
                    can_frame->dlc,
                    can_frame->data);

    return 0;
}

void can1_rx0_isr(void)
{


    // Sicherstellen, dass wirklich eine Nachricht anliegt
    if ((CAN_RF0R(CAN1) & CAN_RF0R_FMP0_MASK) != 0) {
        struct can_rx_msg can_frame;
        ss_can_read(1, &can_frame);
        fifo_add_can_frame(&can_receive_fifos[0], &can_frame);
    }

    // Eventuell: FIFO Overrun Flag löschen
    if (CAN_RF0R(CAN1) & CAN_RF0R_FOVR0) {
        CAN_RF0R(CAN1) &= ~CAN_RF0R_FOVR0; // Flag löschen
        // Optional: Log / Fehlerzähler
    }

}

void can1_sce_isr(void)
{
    // Fehlerflags auslesen
    uint32_t esr = CAN_ESR(CAN1);
    uint32_t msr = CAN_MSR(CAN1);

    // Optional: Fehler zählen, loggen, resetten...
    (void)esr;
    (void)msr;

    // Flags löschen
    CAN_ESR(CAN1) &= ~(CAN_ESR_BOFF | CAN_ESR_EPVF | CAN_ESR_EWGF);
}




void init_fifo(struct Fifo* fifo) {
    if (fifo != NULL) {
        fifo->front = -1;
        fifo->rear = -1;
    }
}

uint8_t is_fifo_empty(struct Fifo* fifo) {
    return fifo != NULL && fifo->front == -1;
}

uint8_t is_fifo_full(struct Fifo* fifo) {
    return fifo != NULL && fifo->rear == FIFO_SIZE - 1;
}

int8_t fifo_add_can_frame(struct Fifo* fifo, struct can_rx_msg* can_frame) {
    if (fifo == NULL || can_frame == NULL) return -1;  // Überprüfung auf Null-Zeiger

    // Wenn die FIFO voll ist, gibt es keinen Platz
    if (is_fifo_full(fifo)) return -1;

    // Wenn die FIFO leer ist, setze den front-Index auf 0
    if (fifo->front == -1) fifo->front = 0;

    // Erhöhe den rear-Index (gehe zum nächsten freien Platz)
    if (fifo->rear < FIFO_SIZE - 1) {
        fifo->rear++;
    } else {
        return -1;
    }
    

    

    // Füge das CAN-Frame hinzu
    fifo->can_frames[fifo->rear].std_id = can_frame->std_id;
    fifo->can_frames[fifo->rear].ext_id = can_frame->ext_id;
    fifo->can_frames[fifo->rear].ide = can_frame->ide;
    fifo->can_frames[fifo->rear].rtr = can_frame->rtr;
    fifo->can_frames[fifo->rear].dlc = can_frame->dlc;
    fifo->can_frames[fifo->rear].fmi = can_frame->fmi;

    // Kopiere die Daten (maximal 8 Bytes, je nach dlc)
    for (int i = 0; i < can_frame->dlc; i++) {
        fifo->can_frames[fifo->rear].data[i] = can_frame->data[i];
    }

    return 0;
}

int8_t fifo_remove_can_frame(struct Fifo* fifo, struct can_rx_msg* can_frame) {
    if (fifo == NULL || can_frame == NULL) return -1;  // Überprüfung auf Null-Zeiger

    // Wenn die FIFO leer ist, gibt es kein Frame zu entfernen
    if (is_fifo_empty(fifo)) return -1;

    //if (fifo->front > FIFO_SIZE - 1) return -1;

    // Kopiere das CAN-Frame vom front der FIFO in den übergebenen Buffer
    can_frame->std_id = fifo->can_frames[fifo->front].std_id;
    can_frame->ext_id = fifo->can_frames[fifo->front].ext_id;
    can_frame->ide = fifo->can_frames[fifo->front].ide;
    can_frame->rtr = fifo->can_frames[fifo->front].rtr;
    can_frame->dlc = fifo->can_frames[fifo->front].dlc;
    can_frame->fmi = fifo->can_frames[fifo->front].fmi;

    // Kopiere die Daten (maximal 8 Bytes, je nach dlc)
    for (int i = 0; i < can_frame->dlc; i++) {
        can_frame->data[i] = fifo->can_frames[fifo->front].data[i];
        fifo->can_frames[fifo->front].data[i] = 0;  // Lösche die Daten im FIFO
    }

    // Wenn front == rear, FIFO ist nun leer
    if (fifo->front == fifo->rear) {
        fifo->front = -1;
        fifo->rear = -1;
    } else {
        // Bewege den front-Index zum nächsten Element
        fifo->front++;
        if (fifo->front > FIFO_SIZE - 1) fifo->front = 0;
    }

    return 0;
}


void ss_can_init_timeout_detection(struct TimeOutDetection* tod) {
    tod->msg_count = 0;
}

uint8_t ss_can_add_timeout(struct TimeOutDetection* tod, uint32_t id, uint16_t reset_value) {
    uint8_t ret = 1;

    tod->msgs[tod->msg_count].std_id = id;
    tod->msgs[tod->msg_count].active_counter = reset_value;
    tod->msgs[tod->msg_count].reset_value = reset_value;
    tod->msgs[tod->msg_count].timeout_detected = 0;

    if (tod->msg_count > MAX_TIMEOUT_DETECTION) {
        ret = 0;
    } else {
        tod->msg_count++;
    }
    
    return ret;
}

uint8_t ss_can_check_timeout_detection(struct TimeOutDetection* tod) {
    uint8_t ret = 0;
    for (uint8_t i = 0; i < tod->msg_count; i++) {
        if (tod->msgs[i].active_counter == 0) {
            ret = 1;
            tod->msgs[i].timeout_detected = 1;
        } else {
            tod->msgs[i].timeout_detected = 0;
            tod->msgs[i].active_counter--;
        }
    }
    return ret;
}

void ss_can_update_timeout_detection(struct TimeOutDetection* tod, uint32_t id) {
    for (uint8_t i = 0; i < tod->msg_count; i++) {
        if (tod->msgs[i].std_id == id) {
            tod->msgs[i].active_counter = tod->msgs[i].reset_value;
        }
    }
}