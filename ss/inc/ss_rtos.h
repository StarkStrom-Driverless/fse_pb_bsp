/**
 * @author  Maximilian Hoffmann <m.hoffmann@startstrom.de>
 * @company Startstrom Augsburg
 * @mail    <maximilian.hoffmann@startstrom-augsburg.de>
 *
 * Copyright (c) 2025 Startstrom Augsburg
 * All rights reserved.
 */

#include "ss_config.h"

#if COMPILE_SS_RTOS

#ifndef _SS_RTOS_H_
#define _SS_RTOS_H_

#include "FreeRTOS.h"
#include "task.h"
#include "ss_feedback.h"

#define SS_RTOS_DELAY_MS pdMS_TO_TICKS

#define SS_RTOS_TASK_ADD(func, prio) ss_rtos_add_task_generic(func, NULL, prio, #func, configMINIMAL_STACK_SIZE)

SS_FEEDBACK ss_rtos_task_add(  TaskFunction_t task_ptr,
                        void * const params,
                        UBaseType_t prio,
                        char* name);

SS_FEEDBACK ss_rtos_big_task_add(  TaskFunction_t task_ptr,
                        void * const params,
                        UBaseType_t prio,
                        char* name);

#ifdef USE_PRIVATE
SS_FEEDBACK ss_rtos_add_task_generic(   TaskFunction_t task_ptr,
                                        void *const params,
                                        UBaseType_t prio,
                                        char* name,
                                        size_t stack_size);
#endif

void ss_rtos_start(void);

void ss_rtos_delay_ms(const uint32_t delay_ms);

void ss_rtos_delay_s(const uint32_t delay_s);

#endif // _SS_RTOS_H_

#endif // COMPILE_SS_RTOS
