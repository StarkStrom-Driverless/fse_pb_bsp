#ifndef _SS_RTOS_H_
#define _SS_RTOS_H_

#include "FreeRTOS.h"
#include "task.h"

#define SS_RTOS_DELAY_MS pdMS_TO_TICKS

int8_t ss_rtos_task_add(  TaskFunction_t task_ptr,
                        void * const params, 
                        UBaseType_t prio,
                        const char* name);

int8_t ss_rtos_rx_task_add( TaskFunction_t task_ptr, 
                            void *const params, 
                            UBaseType_t prio, 
                            const char* name, 
                            TaskHandle_t *task_handle);

int8_t ss_rtos_big_task_add(  TaskFunction_t task_ptr,
                        void * const params, 
                        UBaseType_t prio,
                        const char* name);                    

void ss_rtos_start();

void ss_rtos_delay_ms(const uint32_t delay_ms);

void ss_rtos_delay_s(const uint32_t delay_s);

#endif