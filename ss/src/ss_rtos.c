/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#define USE_PRIVATE
#include "ss_config.h"

#if COMPILE_SS_RTOS
#include "ss_rtos.h"
#include "ss_fsm.h"
#include "ss_error.h"

#define SS_FEEDBACK_BASE SS_FEEDBACK_BASE_NOT_SET

#define SS_RTOS_MAX_TASKS 16

static struct SS_ERROR_CONTEXT ss_error_pool[SS_RTOS_MAX_TASKS];
static uint8_t                 ss_error_pool_used = 0;

SS_FEEDBACK ss_rtos_add_task_generic(TaskFunction_t task_ptr, void *const params, UBaseType_t prio, char* name, size_t stack_size) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    if (prio >= configMAX_PRIORITIES) {
        rc = SS_FEEDBACK_RTOS_INIT_TASK_ERROR;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);


    TaskHandle_t task_handle = NULL;
    BaseType_t task_created = xTaskCreate (task_ptr,name,stack_size,params,prio,&task_handle);
    if (task_created != pdPASS) {
        rc = SS_FEEDBACK_RTOS_INIT_TASK_ERROR;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    if (ss_error_pool_used < SS_RTOS_MAX_TASKS) {
        ss_error_register_task(task_handle, &ss_error_pool[ss_error_pool_used++]);
    }

    rc = ss_fsm_eventqueue_add(name);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    return SS_FEEDBACK_OK;
}

SS_FEEDBACK ss_rtos_task_add(TaskFunction_t task_ptr, void *const params, UBaseType_t prio, char* name) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    rc = ss_rtos_add_task_generic(task_ptr, params, prio, name, 1024);

    return rc;
}



SS_FEEDBACK ss_rtos_big_task_add(TaskFunction_t task_ptr, void *const params, UBaseType_t prio, char* name) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    rc = ss_rtos_add_task_generic(task_ptr, params, prio, name, 2048);

    return rc;
}

SS_FEEDBACK ss_rtos_task_delete(char* name) {
    TaskHandle_t h = xTaskGetHandle(name);
    if (h == NULL) {
        return SS_FEEDBACK_ERROR;
    }
    vTaskDelete(h);
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
#endif // COMPILE_SS_RTOS
