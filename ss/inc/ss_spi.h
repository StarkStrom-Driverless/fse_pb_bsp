/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#ifndef _SS_SPI_H_
#define _SS_SPI_H_

#include <inttypes.h>
#include "ss_feedback.h"

SS_FEEDBACK ss_enable_spi_gpios(uint8_t spi_interface_id);

SS_FEEDBACK ss_enable_spi_rcc(uint8_t can_interface_id);

uint32_t ss_get_spi_port_from_id(uint8_t spi_interface_id);



SS_FEEDBACK ss_spi_init(uint8_t spi_interface_id, uint32_t baudrate);

int8_t ss_spi_rxtx(uint8_t spi_interface_id, uint8_t* rx, uint8_t* tx, uint16_t num);

#endif