#include "ss_eth.h"

#include <stddef.h>
#include <string.h>
#include "ss_gpio.h"
#include "ss_spi.h"
#include "ss_delay.h"
#include "wizchip_conf.h"
#include "socket.h"

#define SS_FEEDBACK_BASE  SS_FEEDBACK_ETH_INIT_ERROR


static uint8_t mem_tx[8] = {2,2,2,2,2,2,2,2};
static uint8_t mem_rx[8] = {2,2,2,2,2,2,2,2};

struct WZ5500 ss_eth;


static void wizchip_cs_select(void) {
    ss_io_write(ss_eth.cs_pin_id, SS_GPIO_OFF);
}

static void wizchip_cs_deselect(void) {
    ss_io_write(ss_eth.cs_pin_id, SS_GPIO_ON);
}

static void wizchip_spi_write(uint8_t wb) {
    uint8_t rx;
    ss_spi_rxtx(W5500_SPI_ID, &rx, &wb, 1);
}

static uint8_t wizchip_spi_read(void) {
    uint8_t tx = 0xFF, rx;
    ss_spi_rxtx(W5500_SPI_ID, &rx, &tx, 1);
    return rx;
}



static void wizchip_spi_readburst(uint8_t* pBuf, uint16_t len) {
    static uint8_t dummy[1536];
    if (len > sizeof(dummy)) len = sizeof(dummy);
    for (uint16_t i=0; i<len; i++) dummy[i] = 0xFF;
    ss_spi_rxtx(W5500_SPI_ID, pBuf, dummy, len);
}

static void wizchip_spi_writeburst(uint8_t* pBuf, uint16_t len) {
    static uint8_t sink[1536];
    if (len > sizeof(sink)) len = sizeof(sink);
    ss_spi_rxtx(W5500_SPI_ID, sink, pBuf, len);
}


SS_FEEDBACK ss_eth_cpy_ip_style(uint8_t* dest, uint64_t source, uint8_t len) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    uint8_t *tmp = (uint8_t*)&source;

    for(uint8_t i = 0; i < len; i++) {
        dest[i] = tmp[i];
    }

    return rc;
}

SS_FEEDBACK ss_eth_set_gw(uint32_t gw) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    ss_eth_cpy_ip_style(    ss_eth.intf_conf.gw, 
                            gw, 
                            4);

    return rc;
}

SS_FEEDBACK ss_eth_set_nm(uint32_t nm) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    ss_eth_cpy_ip_style(    ss_eth.intf_conf.nm, 
                            nm, 
                            4);

    return rc;
}

SS_FEEDBACK ss_eth_set_dns(uint32_t dns) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    ss_eth_cpy_ip_style(    ss_eth.intf_conf.dns, 
                            dns, 
                            4);

    return rc;
}

SS_FEEDBACK ss_eth_set_mac(uint64_t mac) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    ss_eth_cpy_ip_style(    ss_eth.intf_conf.mac, 
                            mac, 
                            6);

    return rc;
}


SS_FEEDBACK ss_eth_init(uint32_t ip, uint32_t sn) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    ss_eth.cs_pin_id = PIN('A', 4);
    ss_eth.rst_pin_id = PIN('B', 10);

    ss_eth.baudrate = 1312500;

    ss_eth_cpy_ip_style(    ss_eth.intf_conf.gw,
                            SS_ETH_IP(192, 168, 10, 1),
                            4);

    ss_eth_cpy_ip_style(    ss_eth.intf_conf.nm,
                            sn,
                            4);

    ss_eth_cpy_ip_style(    ss_eth.intf_conf.ip,
                            ip,
                            4);         

    ss_eth.ports.insert_pos = 0;
    
    rc = ss_io_init(ss_eth.cs_pin_id, SS_GPIO_MODE_OUTPUT);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_io_init(ss_eth.rst_pin_id, SS_GPIO_MODE_OUTPUT);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    ss_io_write(ss_eth.cs_pin_id, SS_GPIO_ON);

    if (ss_spi_init(W5500_SPI_ID, ss_eth.baudrate) != SS_FEEDBACK_OK) {
        return SS_SET_TOPLEVEL_ERROR(SS_FEEDBACK_ETHERNET_INIT_ERROR, SS_FEEDBACK_SPI_INIT_ERROR);
    }

    rc = ss_eth_init_wiz();
    
    return rc;
}

SS_FEEDBACK ss_eth_init_wiz() {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    ss_io_write(ss_eth.rst_pin_id, SS_GPIO_OFF);
    ss_delay(1000);
    ss_io_write(ss_eth.rst_pin_id, SS_GPIO_ON);

    reg_wizchip_cs_cbfunc(wizchip_cs_select, wizchip_cs_deselect);
    reg_wizchip_spi_cbfunc(wizchip_spi_read, wizchip_spi_write);

    if (wizchip_init(mem_tx, mem_rx) != 0) {
        rc = SS_FEEDBACK_ETHERNET_WIZ_INIT_ERROR;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    static wiz_NetInfo netinfo;
    memcpy(netinfo.mac, ss_eth.intf_conf.mac, sizeof(uint8_t) * 6);
    memcpy(netinfo.ip, ss_eth.intf_conf.ip, sizeof(uint8_t) * 4);
    memcpy(netinfo.sn, ss_eth.intf_conf.nm, sizeof(uint8_t) * 4);
    memcpy(netinfo.gw, ss_eth.intf_conf.gw, sizeof(uint8_t) * 4);
    memcpy(netinfo.dns, ss_eth.intf_conf.dns, sizeof(uint8_t) * 4);
    netinfo.dhcp = NETINFO_STATIC;

    ctlnetwork(CN_SET_NETINFO, (void*)&netinfo);

    static wiz_PhyConf pc = { 
        .by = PHY_CONFBY_SW, 
        .mode = PHY_MODE_MANUAL,
        .speed = PHY_SPEED_10, 
        .duplex = PHY_DUPLEX_FULL 
    };
    ctlwizchip(CW_SET_PHYCONF, &pc);
    ctlwizchip(CW_RESET_PHY, 0);
    ss_delay(1000);

    uint8_t ver = getVERSIONR();
    if (getVERSIONR() != 0x04) {
        rc = SS_FEEDBACK_ETHERNET_WIZ_INIT_ERROR;
    }

    return rc;
}



SS_FEEDBACK ss_eth_socket_udp_add(uint32_t port, struct SS_ETH_PAYLOAD* payload) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    struct SS_ETH_INTF *port_ptr = &ss_eth.ports.port[ss_eth.ports.insert_pos];

    port_ptr->intf_number = ss_eth.ports.insert_pos;
    port_ptr->intf_type = Sn_MR_UDP;
    port_ptr->intf_flags = SF_IO_NONBLOCK;
    port_ptr->port = port;
    port_ptr->payload = payload;

    if (ss_eth.ports.insert_pos++ == SS_ETH_MAX_PORTS) {
        rc = SS_FEEDBACK_ETH_MAX_PORTS;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    socket(port_ptr->intf_number, Sn_MR_UDP, port, port_ptr->intf_flags);

    return rc;
}

SS_FEEDBACK ss_eth_get(uint32_t port, struct SS_ETH_INTF** tmp) {
    SS_FEEDBACK rc = SS_FEEDBACK_ETH_PORT_NOT_FOUND;

    for (uint8_t i = 0; i < SS_ETH_MAX_PORTS; i++) {
        if (ss_eth.ports.port[i].port == port) {
            rc = SS_FEEDBACK_OK;
            *tmp = &ss_eth.ports.port[i];
            break;
        }
    }

    return rc;
}

SS_FEEDBACK ss_eth_read(struct SS_ETH_INTF* tmp, struct SS_ETH_PAYLOAD** payload) {
    SS_FEEDBACK rc = SS_FEEDBACK_ETH_NO_MSG_RECEIVED;

    uint32_t len = recvfrom(    tmp->intf_number,
                                tmp->payload->buffer,
                                tmp->payload->buffer_len,
                                tmp->payload->id.ip,
                                &tmp->payload->id.port);

    *payload = tmp->payload;

    if (len <= 0) {
        rc = SS_FEEDBACK_ETH_MSG_RECEIVED;
    }

    return rc;
}

SS_FEEDBACK ss_eth_send(struct SS_ETH_INTF* tmp, struct SS_ETH_PAYLOAD* payload) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    uint8_t len = sendto(   tmp->intf_number,
                            payload->buffer,
                            payload->buffer_len,
                            payload->id.ip,
                            payload->id.port);
    if (len <= 0) {
        rc = SS_FEEDBACK_ETH_TRANSMIT_ERROR;
    }
    return rc;
}



