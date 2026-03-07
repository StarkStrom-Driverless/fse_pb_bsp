#define USE_PRIVATE
#include "ss_config.h"

#if COMPILE_SS_FSM
#include "ss_fsm.h"

#ifndef typeof
#define typeof __typeof__
#endif

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include <stdbool.h>

#include "ss_rtos.h"

#define SS_FEEDBACK_BASE SS_FEEDBACK_BASE_NOT_SET

SS_FSM_EVENTQUEUE_HASHMAP_t* ss_fsm_eventqueue_hashmap = NULL;
SS_FSM_EVENTQUEUE_HASHMAP_t* ss_fsm_eventqueue_prio_hashmap = NULL;
SS_FSM_EVENT_HASHMAP_t* ss_fsm_event_hashmap = NULL;


SS_FEEDBACK ss_fsm_eventqueue_add(char* name) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    QueueHandle_t queue = NULL;
    QueueHandle_t queue_prio = NULL;
    static bool init_core = false;
    
    if (init_core == false) {
        init_core = true;
        rc = ss_fsm_eventqueue_add(SS_FSM_CORE);
        SS_HANDLE_ERROR_WITH_EXIT(rc);
    }

    queue = xQueueCreate(20, sizeof(int32_t));
    if (queue == NULL) {
        rc = SS_FEEDBACK_FSM_INIT_ERROR;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    shput(ss_fsm_eventqueue_hashmap, name, queue);


    queue_prio = xQueueCreate(20, sizeof(int32_t));
    if(queue_prio == NULL) {
        rc = SS_FEEDBACK_FSM_INIT_ERROR;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    shput(ss_fsm_eventqueue_prio_hashmap, name, queue_prio);


    return rc;
}

SS_FEEDBACK ss_fsm_eventqueue_get(char* name, QueueHandle_t* handle, QueueHandle_t* handle_prio) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    QueueHandle_t queue;
    QueueHandle_t queue_prio;

    queue = shget(ss_fsm_eventqueue_hashmap, name);
    if (queue == NULL) {
        rc = SS_FEEDBACK_FSM_WRONG_KEY;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);
    *handle = queue;

    queue_prio = shget(ss_fsm_eventqueue_prio_hashmap, name);
    if (queue == NULL) {
        rc = SS_FEEDBACK_FSM_WRONG_KEY; 
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);
    *handle_prio = queue_prio;

    return rc;
}

int32_t ss_fsm_event_receive(char *name) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    QueueHandle_t queue;
    QueueHandle_t queue_prio;
    char *callback_name = NULL;
    int32_t value;
    UBaseType_t waiting_msgs = 0;


    if (name == NULL) {
        callback_name = pcTaskGetName(NULL);
    } else {
        callback_name = name;
    }

    rc = ss_fsm_eventqueue_get(callback_name, &queue, &queue_prio);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    waiting_msgs = uxQueueMessagesWaiting(queue_prio);

    if (waiting_msgs == 0) {
        BaseType_t tmp = xQueueReceive(queue, &value, ( TickType_t ) 0 ); 
        if (tmp != pdPASS) {
            value = -1;
        }
    } else {
        BaseType_t tmp = xQueueReceive(queue_prio, &value, ( TickType_t ) 0 ); 
        if (tmp != pdPASS) {
            value = -1;
        }
    }

    return value;
}

SS_FEEDBACK ss_fsm_event_send_to(char *name, int32_t value, bool prio) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    QueueHandle_t queue;
    QueueHandle_t queue_prio;
    int32_t send_value = value;
    BaseType_t tmp = pdPASS;

    rc = ss_fsm_eventqueue_get(name, &queue, &queue_prio);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    if (prio){
        tmp = xQueueSend(queue_prio, &send_value, (TickType_t) 0);
    } else {
        tmp = xQueueSend(queue, &send_value, (TickType_t) 0);
    }
        
    if (tmp != pdPASS) {
        rc = SS_FEEDBACK_FSM_EVENT_SEND_FAILED;
    }

    return rc;
}

SS_FEEDBACK ss_fsm_event_send(int32_t event) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    char* name;

    rc = ss_fsm_event_get(RM_PRIO(event), &name);
    SS_HANDLE_ERROR_WITH_EXIT(rc);


    rc = ss_fsm_event_send_to(name, RM_PRIO(event), HAS_PRIO(event));

    return rc;
}

SS_FEEDBACK ss_fsm_event_send_core(int32_t value) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    rc = ss_fsm_event_send_to(SS_FSM_CORE, RM_PRIO(value), HAS_PRIO(value));
    return rc;
}


int32_t ss_fsm_event_receive_core() {
    int32_t rc = -1;
    rc = ss_fsm_event_receive(SS_FSM_CORE);
    return rc;
}

SS_FEEDBACK ss_fsm_event_get(int32_t event_code, char** task_name) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    char *name = NULL;

    name = hmget(ss_fsm_event_hashmap, event_code);
    if (name == NULL) {
        rc = SS_FEEDBACK_FSM_WRONG_KEY;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    *task_name = name;

    return rc;
}

SS_FEEDBACK ss_fsm_event_add(int32_t event_code) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    char* task_name = pcTaskGetName(NULL);

    hmput(ss_fsm_event_hashmap, event_code, task_name);

    return rc;
}


#endif // COMPILE_SS_FSM
