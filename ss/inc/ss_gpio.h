#ifndef __SS_GPIO_H__
#define __SS_GPIO_H__

#include <inttypes.h>

#include "ss_makros.h"

#define GPIO(bank) ((uint32_t) (0x40020000 + 0x400 * (bank)))

enum { SS_GPIO_MODE_INPUT, SS_GPIO_MODE_OUTPUT, SS_GPIO_MODE_AF, SS_GPIO_MODE_ANALOG, SS_GPIO_MODE_PWM, SS_GPIO_MODE_INPUT_PU, SS_GPIO_MODE_INPUT_PD};
enum {SS_GPIO_OFF, SS_GPIO_ON, SS_GPIO_TOGGLE};

int8_t ss_enable_rcc_from_id(uint16_t pin_id);

uint16_t ss_io_init(uint16_t pin_id, uint8_t mode);

void ss_io_write(uint16_t pin_id, uint8_t value);

uint16_t ss_io_read(uint16_t pin_id);



#endif

