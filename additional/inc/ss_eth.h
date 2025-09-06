#ifndef _WZ5500_WRAPPER_H_
#define _WZ5500_WRAPPER_H_

#include <stdint.h>

#include "wizchip_conf.h"
#include "socket.h"
#include "ss_feedback.h"

#define W5500_SPI_ID 1

struct WZ5500 {
    uint16_t cs_pin_id;
    uint16_t rst_pin_id;
};

extern struct WZ5500 wz5500;

SS_FEEDBACK ss_eth_init(wiz_NetInfo* netinfo, uint32_t spi_baudrate);

void ss_eth_init_udp_socket(uint32_t port);

#endif
