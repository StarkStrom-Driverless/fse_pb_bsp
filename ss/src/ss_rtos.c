#include "ss_rtos.h"





SS_FEEDBACK ss_rtos_task_add(TaskFunction_t task_ptr, void *const params, UBaseType_t prio, const char* name) {
    if (prio < configMAX_PRIORITIES) {
        BaseType_t task_created = xTaskCreate (task_ptr,name,1024,params,prio,NULL);
        if (task_created != pdPASS) {
            return SS_FEEDBACK_RTOS_INIT_TASK_ERROR;
        }
    } else {
        return SS_FEEDBACK_RTOS_INIT_TASK_ERROR;
    }
    return SS_FEEDBACK_OK;
}

SS_FEEDBACK ss_rtos_rx_task_add(TaskFunction_t task_ptr, void *const params, UBaseType_t prio, const char* name, TaskHandle_t* task_handle) {
    if (prio < configMAX_PRIORITIES) {
        BaseType_t task_created = xTaskCreate (task_ptr,name,1024,params,prio,task_handle);
        if (task_created != pdPASS) {
            return SS_FEEDBACK_RTOS_INIT_RX_TASK_ERROR;
        }
    } else {
        return SS_FEEDBACK_RTOS_INIT_RX_TASK_ERROR;
    }
    return SS_FEEDBACK_OK;
}

SS_FEEDBACK ss_rtos_big_task_add(TaskFunction_t task_ptr, void *const params, UBaseType_t prio, const char* name) {
    if (prio < configMAX_PRIORITIES) {
        BaseType_t task_created = xTaskCreate (task_ptr,name,8196,params,prio,NULL);
        if (task_created != pdPASS) {
            return SS_FEEDBACK_RTOS_INIT_BIGTASK_ERROR;
        }
    } else {
        return SS_FEEDBACK_RTOS_INIT_BIGTASK_ERROR;
    }
    return SS_FEEDBACK_OK;
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