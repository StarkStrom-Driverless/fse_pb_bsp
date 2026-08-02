#define USE_PRIVATE
#include "ss.h"

struct SS_ETH_PAYLOAD payload;

void example_task(void* args) {
    struct SS_ETH_PAYLOAD* payload_ptr;
    struct SS_ETH_INTF* intf;

    ss_eth_get(6301, &intf);

    ss_eth_cpy_ip_style(payload.id.ip, SS_ETH_IP(192, 168, 10, 122), 4);
    payload.id.port = 6301;

    for (;;) {
        payload.buffer[0] = 0xA;
        payload.buffer[1] = 0xF;
        payload.buffer[2] = 0xF;
        payload.buffer[3] = 0xE;
        payload.buffer_len = 4;

        ss_eth_send(intf, &payload);

        ss_printf(4, "ETH: sent \r\n");

        if (ss_eth_read(intf, &payload_ptr)) {
            ss_led_heartbeat_toggle();
            ss_printf(4, "ETH: received %d bytes \r\n", payload_ptr->received_len);
        }

        ss_rtos_delay_ms(1000);
    }
}

int main(void) {
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_uart_init(4, 115200));

    SS_ERROR_ASSERT(ss_eth_init(
        SS_ETH_IP(192, 168, 10, 104),
        SS_ETH_IP(255, 255, 0, 0),
        SS_ETH_MAC(0x00, 0x08, 0xDC, 0xAB, 0xCD, 0xEF),
        SS_ETH_IP(192, 168, 10, 1)
    ));

    SS_ERROR_ASSERT(ss_eth_socket_udp_add(6301, &payload));

    ss_printf(4, "fse_pb_bsp example: eth_udp \r\n");

    SS_ERROR_ASSERT(ss_rtos_task_add(example_task, NULL, 1, "example_task"));

    ss_rtos_start();

    while (1) {
    }

    return 0;
}

void ss_error_fail(void) {
    while (1) {
        ss_led_error_toggle();
        ss_delay(1000);
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName) {
    while(1) {
        ss_led_error_toggle();
        ss_delay(500);
    }
}
