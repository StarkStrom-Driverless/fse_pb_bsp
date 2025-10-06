#ifndef _SS_FEEDBACK_H_
#define _SS_FEEDBACK_H_

#include <inttypes.h>

typedef uint32_t SS_FEEDBACK;

 

#define SS_FEEDBACK_OK                              (SS_FEEDBACK)0x00
#define SS_FEEDBACK_ERROR                           (SS_FEEDBACK)0x01
#define SS_FEEDBACK_NULL                            (SS_FEEDBACK)0x02
#define SS_FEEDBACK_BASE_NOT_SET                    (SS_FEEDBACK)0x03

#define SS_FEEDBACK_CLOCK_INIT_ERROR                (SS_FEEDBACK)0x10
#define SS_FEEDBACK_CLOCK_CAN_INIT_ERROR            (SS_FEEDBACK)0x11
#define SS_FEEDBACK_CLOCK_SPI_INIT_ERROR            (SS_FEEDBACK)0x12

#define SS_FEEDBACK_SPI_INIT_ERROR                  (SS_FEEDBACK)0x20
#define SS_FEEDBACK_SPI_GPIO_INIT_ERROR             (SS_FEEDBACK)0x21
#define SS_FEEDBACK_SPI_RCC_INIT_ERROR              (SS_FEEDBACK)0x22

#define SS_FEEDBACK_RCC_INIT_ERROR                  (SS_FEEDBACK)0x30

#define SS_FEEDBACK_ETHERNET_INIT_ERROR             (SS_FEEDBACK)0x40
#define SS_FEEDBACK_ETHERNET_WIZ_INIT_ERROR         (SS_FEEDBACK)0x40

#define SS_FEEDBACK_IO_INIT_ERROR                   (SS_FEEDBACK)0x50
#define SS_FEEDBACK_IO_PB_LEDS_INIT_ERROR           (SS_FEEDBACK)0x51
#define SS_FEEDBACK_IO_PWM_INIT_ERROR               (SS_FEEDBACK)0x52
#define SS_FEEDBACK_IO_PINID_ERROR                  (SS_FEEDBACK)0x53
#define SS_FEEDBACK_IO_IOB_ERROR                    (SS_FEEDBACK)0x54
#define SS_FEEDBACK_IO_IOB_PINID_OFR                (SS_FEEDBACK)0x55

#define SS_FEEDBACK_RTOS_INIT_TASK_ERROR            (SS_FEEDBACK)0x60
#define SS_FEEDBACK_RTOS_INIT_RX_TASK_ERROR         (SS_FEEDBACK)0x61
#define SS_FEEDBACK_RTOS_INIT_BIGTASK_ERROR         (SS_FEEDBACK)0x62

#define SS_FEEDBACK_BASE                            SS_FEEDBACK_BASE_NOT_SET

#define SS_HANDLE_INIT(func)                        if (func != SS_FEEDBACK_OK) ss_init_error(func)
#define SS_SET_TOPLEVEL_ERROR(err, tpl_err)         (SS_FEEDBACK)(tpl_err << 16 | err)
#define SS_HANDLE_ERROR_WITH_EXIT(rc)               if (rc != SS_FEEDBACK_OK) return SS_SET_TOPLEVEL_ERROR(SS_FEEDBACK_BASE, rc)
#define SS_HANDLE_NULL_WITH_EXIT(value)             if (value == 0) return SS_SET_TOPLEVEL_ERROR(SS_FEEDBACK_BASE, SS_FEEDBACK_NULL)

#endif