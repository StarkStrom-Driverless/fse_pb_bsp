#ifndef _WZ5500_WRAPPER_H_
#define _WZ5500_WRAPPER_H_

#include <stdint.h>

#include "wizchip_conf.h"
#include "socket.h"
#include "ss_feedback.h"

#define W5500_SPI_ID 1

#define SS_ETH_UDP 0
#define SS_ETH_TCP 1
#define SS_ETH_ETH 2

#define SS_ETH_MAX_PORTS 8

#define SS_ETH_PAYLOAD_BUFFER_SIZE 1500

#define SS_ETH_IP(a, b, c, d)           (uint32_t)((a << 24) | (b << 16) | (c << 8) | d)
#define SS_ETH_MAC(a, b, c, d, e, f)    (uint64_t)((a << 40) | (b << 32) | (c << 24) | (d << 16) | (e << 8) | f)
#define SS_ETH_IP_GET_POS(ip, pos)      (0xFF & (a >> pos * 8))

struct SS_ETH_SENDER {
    uint8_t ip[4];
    uint32_t port;
};

struct SS_ETH_ID {
    uint8_t ip[4];
    uint32_t port;
};

struct SS_ETH_PAYLOAD {
    uint8_t buffer[SS_ETH_PAYLOAD_BUFFER_SIZE];
    uint16_t buffer_len;
    struct SS_ETH_ID id;
};

struct SS_ETH_INTF_CONF {
    uint8_t gw[4];
    uint8_t nm[4];
    uint8_t ip[4];
    uint8_t dns[4];
    uint8_t mac[6];
};

struct SS_ETH_INTF {
    uint32_t intf_number;
    uint8_t intf_type;
    uint8_t intf_flags;
    uint32_t port;
    struct SS_ETH_PAYLOAD* payload;
};



struct SS_ETH_PORTS {
    struct SS_ETH_INTF port[SS_ETH_MAX_PORTS];
    uint8_t insert_pos;
};

struct WZ5500 {
    uint16_t cs_pin_id;
    uint16_t rst_pin_id;
    uint32_t baudrate;

    struct SS_ETH_INTF_CONF intf_conf; 

    struct SS_ETH_PORTS ports;
};

extern struct WZ5500 ss_eth;

/***
 * 
 * SUPPORT FUNCTIONS
 * 
 */
SS_FEEDBACK ss_eth_cpy_ip_style(uint8_t* dest, uint64_t source, uint8_t len);
SS_FEEDBACK ss_eth_init_wiz();

/***
 * 
 * USER FUNCTIONS FOR INIT
 * 
 */
SS_FEEDBACK ss_eth_init(uint32_t ip, uint32_t sn);
SS_FEEDBACK ss_eth_set_gw(uint32_t gw);
SS_FEEDBACK ss_eth_set_nm(uint32_t nm);
SS_FEEDBACK ss_eth_set_dns(uint32_t dns);
SS_FEEDBACK ss_eth_set_mac(uint64_t mac);
SS_FEEDBACK ss_eth_socket_udp_add(uint32_t port, struct SS_ETH_PAYLOAD* payload);

/***
 * 
 * USER FUNCTIONS FOR PROCESS
 * 
 */
SS_FEEDBACK ss_eth_get(uint32_t port, struct SS_ETH_INTF** tmp);
SS_FEEDBACK ss_eth_read(struct SS_ETH_INTF* tmp, struct SS_ETH_PAYLOAD** payload);
SS_FEEDBACK ss_eth_send(struct SS_ETH_INTF* tmp, struct SS_ETH_PAYLOAD* payload);

#endif
