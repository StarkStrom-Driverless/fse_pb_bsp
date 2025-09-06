#ifndef _SS_FEEDBACK_H_
#define _SS_FEEDBACK_H_

#include <inttypes.h>

typedef uint32_t SS_FEEDBACK;

#define SET_TOPLEVEL_ERROR(err, tpl_err)    (SS_FEEDBACK)(tpl_err << 16 | err)

#define SS_FEEDBACK_OK                      (SS_FEEDBACK)0x00
#define SS_FEEDBACK_ERROR                   (SS_FEEDBACK)0x01

#define SS_FEEDBACK_CLOCK_INIT_ERROR        (SS_FEEDBACK)0x10
#define SS_FEEDBACK_CLOCK_CAN_INIT_ERROR    (SS_FEEDBACK)0x11
#define SS_FEEDBACK_CLOCK_SPI_INIT_ERROR    (SS_FEEDBACK)0x12

#define SS_FEEDBACK_SPI_INIT_ERROR          (SS_FEEDBACK)0x20
#define SS_FEEDBACK_SPI_GPIO_INIT_ERROR     (SS_FEEDBACK)0x21

#define SS_FEEDBACK_RCC_INIT_ERROR          (SS_FEEDBACK)0x30

#define SS_FEEDBACK_ETHERNET_INIT_ERROR     (SS_FEEDBACK)0x40
#define SS_FEEDBACK_ETHERNET_WIZ_INIT_ERROR (SS_FEEDBACK)0x40


#define SS_HANDLE_INIT(func)                if (func != SS_FEEDBACK_OK) ss_init_error(func)


#endif