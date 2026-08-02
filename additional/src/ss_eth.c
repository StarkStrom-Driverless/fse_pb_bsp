#define USE_PRIVATE
#include "ss_config.h"

#if COMPILE_SS_ETH
#include "ss_eth.h"

#include <stddef.h>
#include <string.h>
#include "ss_gpio.h"
#include "ss_spi.h"
#include "ss_delay.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "ss_error.h"
#include <libopencm3/stm32/gpio.h>


static uint8_t mem_tx[8] = {2,2,2,2,2,2,2,2};
static uint8_t mem_rx[8] = {2,2,2,2,2,2,2,2};

struct WZ5500 ss_eth;


static void wizchip_cs_select(void) {
    ss_spi_mode(W5500_SPI_ID, 0);
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


bool ss_eth_cpy_ip_style(uint8_t* dest, uint64_t source, uint8_t len) {
    uint8_t *tmp = (uint8_t*)&source;

    for(uint8_t i = 0; i < len; i++) {
        dest[i] = tmp[i];
    }

    return true;
}

bool ss_eth_set_gw(uint32_t gw) {
    ss_eth_cpy_ip_style(    ss_eth.intf_conf.gw,
                            gw,
                            4);

    return true;
}

bool ss_eth_set_nm(uint32_t nm) {
    ss_eth_cpy_ip_style(    ss_eth.intf_conf.nm,
                            nm,
                            4);

    return true;
}

bool ss_eth_set_dns(uint32_t dns) {
    ss_eth_cpy_ip_style(    ss_eth.intf_conf.dns,
                            dns,
                            4);

    return true;
}

bool ss_eth_set_mac(uint64_t mac) {
    ss_eth_cpy_ip_style(    ss_eth.intf_conf.mac,
                            mac,
                            6);

    return true;
}


bool ss_eth_init(uint32_t ip, uint32_t sn, uint64_t mac, uint32_t gw) {
    ss_eth.rst_pin_id = PIN('C', 2);
    ss_eth.cs_pin_id = PIN('A', 10);

    
    ss_eth.baudrate = 2625000;

    ss_eth_cpy_ip_style(    ss_eth.intf_conf.gw,
                            gw,
                            4);

    ss_eth_cpy_ip_style(    ss_eth.intf_conf.nm,
                            sn,
                            4);

    ss_eth_cpy_ip_style(    ss_eth.intf_conf.ip,
                            ip,
                            4);

    ss_eth_cpy_ip_style(    ss_eth.intf_conf.mac,
                            mac,
                            6);



    ss_eth.ports.insert_pos = 0;

    if (!ss_io_init(ss_eth.cs_pin_id, SS_GPIO_MODE_OUTPUT)) SS_ERROR(NULL);

    if (!ss_io_init(ss_eth.rst_pin_id, SS_GPIO_MODE_OUTPUT)) SS_ERROR(NULL);

    gpio_set_output_options(GPIO(PINBANK(ss_eth.cs_pin_id)), GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, BIT(PINNO(ss_eth.cs_pin_id)));
    gpio_set_output_options(GPIO(PINBANK(ss_eth.rst_pin_id)), GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, BIT(PINNO(ss_eth.rst_pin_id)));


    ss_io_write(ss_eth.cs_pin_id, SS_GPIO_ON);

    if (!ss_spi_init(W5500_SPI_ID, ss_eth.baudrate, 0)) SS_ERROR(NULL);

    return ss_eth_init_wiz();
}

bool ss_eth_init_wiz() {
    ss_io_write(ss_eth.rst_pin_id, SS_GPIO_OFF);
    ss_delay(1000);
    ss_io_write(ss_eth.rst_pin_id, SS_GPIO_ON);

    reg_wizchip_cs_cbfunc(wizchip_cs_select, wizchip_cs_deselect);
    reg_wizchip_spi_cbfunc(wizchip_spi_read, wizchip_spi_write);

    if (wizchip_init(mem_tx, mem_rx) != 0) {
        SS_ERROR("wizchip_init failed");
    }

    static wiz_NetInfo netinfo;
    memcpy(netinfo.mac, ss_eth.intf_conf.mac, sizeof(uint8_t) * 6);
    memcpy(netinfo.ip, ss_eth.intf_conf.ip, sizeof(uint8_t) * 4);
    memcpy(netinfo.sn, ss_eth.intf_conf.nm, sizeof(uint8_t) * 4);
    memcpy(netinfo.gw, ss_eth.intf_conf.gw, sizeof(uint8_t) * 4);
    memcpy(netinfo.dns, ss_eth.intf_conf.dns, sizeof(uint8_t) * 4);
    netinfo.dhcp = NETINFO_STATIC;

    ctlnetwork(CN_SET_NETINFO, (void*)&netinfo);

    wiz_PhyConf pc = {
        .by     = PHY_CONFBY_SW,
        .mode   = PHY_MODE_AUTONEGO,   // Auto-Negotiation
        .speed  = PHY_SPEED_100,       // bevorzugt 100M
        .duplex = PHY_DUPLEX_FULL
    };
    ctlwizchip(CW_SET_PHYCONF, &pc);
    ctlwizchip(CW_RESET_PHY, 0);
    ss_delay(1000);

    uint8_t ver = getVERSIONR();
    if (getVERSIONR() != 0x04) {
        SS_ERROR("wizchip version mismatch");
    }


    //ss_eth.baudrate = 10500000;
    ss_eth.baudrate = 42000000;
    if (!ss_spi_init(W5500_SPI_ID, ss_eth.baudrate, 0)) SS_ERROR(NULL);

    return true;
}



bool ss_eth_socket_udp_add(uint32_t port, struct SS_ETH_PAYLOAD* payload) {
    struct SS_ETH_INTF *port_ptr = &ss_eth.ports.port[ss_eth.ports.insert_pos];

    port_ptr->intf_number = ss_eth.ports.insert_pos;
    port_ptr->intf_type = Sn_MR_UDP;
    port_ptr->intf_flags = SF_IO_NONBLOCK;
    port_ptr->port = port;
    port_ptr->payload = payload;

    payload->buffer_len = SS_ETH_PAYLOAD_BUFFER_SIZE;

    if (ss_eth.ports.insert_pos++ == SS_ETH_MAX_PORTS) {
        SS_ERROR("too many eth ports");
    }

    socket(port_ptr->intf_number, Sn_MR_UDP, port, port_ptr->intf_flags);

    return true;
}

bool ss_eth_get(uint32_t port, struct SS_ETH_INTF** tmp) {
    for (uint8_t i = 0; i < SS_ETH_MAX_PORTS; i++) {
        if (ss_eth.ports.port[i].port == port) {
            *tmp = &ss_eth.ports.port[i];
            return true;
        }
    }

    SS_ERROR("eth port not found");
}

bool ss_eth_received_frame(struct SS_ETH_INTF* tmp) {
    return getSn_RX_RSR(tmp->intf_number) > 0;
}

bool ss_eth_read(struct SS_ETH_INTF* tmp, struct SS_ETH_PAYLOAD** payload) {
    if (getSn_RX_RSR(tmp->intf_number) == 0) {
        return false;
    }

    int len = recvfrom(tmp->intf_number,
                       tmp->payload->buffer,
                       tmp->payload->buffer_len,
                       tmp->payload->id.ip,
                       &tmp->payload->id.port);

    tmp->payload->received_len = len;

    *payload = tmp->payload;

    if (len < 0) SS_ERROR("eth recvfrom failed");

    return len > 0;
}

bool ss_eth_read_filtered(struct SS_ETH_INTF* tmp, struct SS_ETH_PAYLOAD** payload, uint16_t expected_len) {
    uint8_t sn = tmp->intf_number;

    if (getSn_RX_RSR(sn) == 0) {
        return false;
    }

    // Read 8-byte W5500 UDP header: [IP:4][Port:2][Len:2]
    uint8_t head[8];
    wiz_recv_data(sn, head, 8);
    setSn_CR(sn, Sn_CR_RECV);
    while (getSn_CR(sn));

    uint16_t pack_len = ((uint16_t)head[6] << 8) | head[7];

    if (pack_len != expected_len) {
        // Discard payload — no SPI data transfer for the actual content
        wiz_recv_ignore(sn, pack_len);
        setSn_CR(sn, Sn_CR_RECV);
        while (getSn_CR(sn));
        return false;
    }

    wiz_recv_data(sn, tmp->payload->buffer, pack_len);
    setSn_CR(sn, Sn_CR_RECV);
    while (getSn_CR(sn));

    tmp->payload->id.ip[0] = head[0];
    tmp->payload->id.ip[1] = head[1];
    tmp->payload->id.ip[2] = head[2];
    tmp->payload->id.ip[3] = head[3];
    tmp->payload->id.port  = ((uint16_t)head[4] << 8) | head[5];
    tmp->payload->received_len = pack_len;
    *payload = tmp->payload;

    return true;
}

bool ss_eth_send(struct SS_ETH_INTF* tmp, struct SS_ETH_PAYLOAD* payload) {
    uint8_t len = sendto(   tmp->intf_number,
                            payload->buffer,
                            payload->buffer_len,
                            payload->id.ip,
                            payload->id.port);
    if (len <= 0) {
        SS_ERROR("eth sendto failed");
    }
    return true;
}

#endif // COMPILE_SS_ETH
