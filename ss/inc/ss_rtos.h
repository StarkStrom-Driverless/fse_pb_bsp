/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 * 
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#ifndef _SS_RTOS_H_
#define _SS_RTOS_H_

#include "FreeRTOS.h"
#include "task.h"
#include "ss_feedback.h"

#define SS_RTOS_DELAY_MS pdMS_TO_TICKS

SS_FEEDBACK ss_rtos_task_add(  TaskFunction_t task_ptr,
                        void * const params, 
                        UBaseType_t prio,
                        char* name);


SS_FEEDBACK ss_rtos_big_task_add(  TaskFunction_t task_ptr,
                        void * const params, 
                        UBaseType_t prio,
                        char* name);      
                        
SS_FEEDBACK ss_rtos_add_task_generic(   TaskFunction_t task_ptr, 
                                        void *const params, 
                                        UBaseType_t prio, 
                                        char* name, 
                                        size_t stack_size);

void ss_rtos_start(void);

void ss_rtos_delay_ms(const uint32_t delay_ms);

void ss_rtos_delay_s(const uint32_t delay_s);

#endif