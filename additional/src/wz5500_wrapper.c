#include "ss_eth.h"

#include <stddef.h>
#include "ss_gpio.h"
#include "ss_spi.h"
#include "ss_delay.h"
#include "wizchip_conf.h"
#include "socket.h"


static uint8_t mem_tx[8] = {2,2,2,2,2,2,2,2};
static uint8_t mem_rx[8] = {2,2,2,2,2,2,2,2};

struct WZ5500 wz5500;


static void wizchip_cs_select(void) {
    ss_io_write(wz5500.cs_pin_id, SS_GPIO_OFF);
}

static void wizchip_cs_deselect(void) {
    ss_io_write(wz5500.cs_pin_id, SS_GPIO_ON);
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

SS_FEEDBACK ss_eth_init(wiz_NetInfo* netinfo, uint32_t spi_baudrate) {
    wz5500.cs_pin_id = PIN('A', 4);
    wz5500.rst_pin_id = PIN('B', 10);
    SS_FEEDBACK rc = 0;

    rc = ss_io_init(wz5500.cs_pin_id, SS_GPIO_MODE_OUTPUT);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_io_init(wz5500.rst_pin_id, SS_GPIO_MODE_OUTPUT);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    ss_io_write(wz5500.cs_pin_id, SS_GPIO_ON);

    ss_io_write(wz5500.rst_pin_id, SS_GPIO_OFF);
    ss_delay(1000);
    ss_io_write(wz5500.rst_pin_id, SS_GPIO_ON);

    if (ss_spi_init(W5500_SPI_ID, spi_baudrate) != SS_FEEDBACK_OK) {
        return SS_SET_TOPLEVEL_ERROR(SS_FEEDBACK_ETHERNET_INIT_ERROR, SS_FEEDBACK_SPI_INIT_ERROR);
    }

    reg_wizchip_cs_cbfunc(wizchip_cs_select, wizchip_cs_deselect);
    reg_wizchip_spi_cbfunc(wizchip_spi_read, wizchip_spi_write);

    if (wizchip_init(mem_tx, mem_rx) != 0) {

    }

    ctlnetwork(CN_SET_NETINFO, (void*)netinfo);

    wiz_PhyConf pc = { 
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
        return SS_FEEDBACK_ETHERNET_WIZ_INIT_ERROR;
    }

    return SS_FEEDBACK_OK;
}

void ss_eth_init_udp_socket(uint32_t port) {
    socket(0, Sn_MR_UDP, port, SF_IO_NONBLOCK);
}



