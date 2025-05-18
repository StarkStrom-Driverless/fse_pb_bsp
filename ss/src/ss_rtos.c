#include "ss_rtos.h"

extern void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName );

void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName) {
	for(;;);	// Loop forever here..
}

int8_t ss_rtos_task_add(TaskFunction_t task_ptr, void *const params, UBaseType_t prio, const char* name) {
    int8_t ret = 1;
    if (prio < configMAX_PRIORITIES) {
        xTaskCreate (task_ptr,name,100,params,prio,NULL);
    } else {
        ret = 0;
    }
    return ret;
}

void ss_rtos_start() {
    vTaskStartScheduler();
}

void ss_rtos_delay_ms(const uint32_t delay_ms) {
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
}

void ss_rtos_delay_s(const uint32_t delay_s) {
    vTaskDelay(pdMS_TO_TICKS(delay_s * 1000));
}