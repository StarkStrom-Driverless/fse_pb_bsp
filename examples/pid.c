#include "ss.h"

float get_measurement(void) {
    return 0.5f;
}

void example_task(void* args) {
    struct SS_PID pid = {0};

    ss_pid_init(&pid);

    pid.ss_pid_period = 0.5f;

    pid.ss_pid_kd = 0.1f;
    pid.ss_pid_kp = 0.8f;
    pid.ss_pid_ki = 0.4f;

    pid.ss_pid_out_max = 1.0f;
    pid.ss_pid_out_min = 0.0f;

    pid.ss_pid_integrator_max = 1.0f;
    pid.ss_pid_integrator_min = -1.0f;

    pid.ss_pid_tau = 0.5f;

    float setpoint = 1.0f;
    float output = 0.0f;

    for (;;) {
        float measurement = get_measurement();

        ss_pid_update(&pid, setpoint, measurement, &output);

        ss_printf(4, "PID: %f \r\n", output);

        ss_rtos_delay_ms(500);
    }
}

int main(void) {
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_uart_init(4, 115200));

    ss_printf(4, "fse_pb_bsp example: pid \r\n");

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
