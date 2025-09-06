#include "ss_rtos.h"





int8_t ss_rtos_task_add(TaskFunction_t task_ptr, void *const params, UBaseType_t prio, const char* name) {
    int8_t ret = 1;
    if (prio < configMAX_PRIORITIES) {
        BaseType_t task_created = xTaskCreate (task_ptr,name,1024,params,prio,NULL);
        if (task_created != pdPASS) {
            ret = 0;
        }
    } else {
        ret = 0;
    }
    return ret;
}

int8_t ss_rtos_rx_task_add(TaskFunction_t task_ptr, void *const params, UBaseType_t prio, const char* name, TaskHandle_t* task_handle) {
    int8_t ret = 1;
    if (prio < configMAX_PRIORITIES) {
        BaseType_t task_created = xTaskCreate (task_ptr,name,1024,params,prio,task_handle);
        if (task_created != pdPASS) {
            ret = 0;
        }
    } else {
        ret = 0;
    }
    return ret;
}

int8_t ss_rtos_big_task_add(TaskFunction_t task_ptr, void *const params, UBaseType_t prio, const char* name) {
    int8_t ret = 1;
    if (prio < configMAX_PRIORITIES) {
        BaseType_t task_created = xTaskCreate (task_ptr,name,8196,params,prio,NULL);
        if (task_created != pdPASS) {
            ret = 0;
        }
    } else {
        ret = 0;
    }
    return ret;
}

void ss_rtos_start(void) {
    vTaskStartScheduler();
}

void ss_rtos_delay_ms(const uint32_t delay_ms) {
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
}

void ss_rtos_delay_s(const uint32_t delay_s) {
    vTaskDelay(pdMS_TO_TICKS(delay_s * 1000));
}