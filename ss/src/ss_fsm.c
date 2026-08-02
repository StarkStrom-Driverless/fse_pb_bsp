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
#include "ss_error.h"

SS_FSM_EVENTQUEUE_HASHMAP_t* ss_fsm_eventqueue_hashmap = NULL;
SS_FSM_EVENTQUEUE_HASHMAP_t* ss_fsm_eventqueue_prio_hashmap = NULL;
SS_FSM_EVENT_HASHMAP_t* ss_fsm_event_hashmap = NULL;


bool ss_fsm_eventqueue_add(char* name) {
    QueueHandle_t queue = NULL;
    QueueHandle_t queue_prio = NULL;
    static bool init_core = false;

    if (init_core == false) {
        init_core = true;
        if (!ss_fsm_eventqueue_add(SS_FSM_CORE)) SS_ERROR(NULL);
    }

    queue = xQueueCreate(20, sizeof(int32_t));
    if (queue == NULL) {
        SS_ERROR("fsm event queue create failed");
    }

    shput(ss_fsm_eventqueue_hashmap, name, queue);


    queue_prio = xQueueCreate(20, sizeof(int32_t));
    if(queue_prio == NULL) {
        SS_ERROR("fsm prio event queue create failed");
    }

    shput(ss_fsm_eventqueue_prio_hashmap, name, queue_prio);


    return true;
}

bool ss_fsm_eventqueue_get(char* name, QueueHandle_t* handle, QueueHandle_t* handle_prio) {
    QueueHandle_t queue;
    QueueHandle_t queue_prio;

    queue = shget(ss_fsm_eventqueue_hashmap, name);
    if (queue == NULL) {
        SS_ERROR("unknown fsm event queue name");
    }
    *handle = queue;

    queue_prio = shget(ss_fsm_eventqueue_prio_hashmap, name);
    if (queue == NULL) {
        SS_ERROR("unknown fsm prio event queue name");
    }
    *handle_prio = queue_prio;

    return true;
}

int32_t ss_fsm_event_receive(char *name) {
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

    if (!ss_fsm_eventqueue_get(callback_name, &queue, &queue_prio)) return -1;

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

bool ss_fsm_event_send_to(char *name, int32_t value, bool prio) {
    QueueHandle_t queue;
    QueueHandle_t queue_prio;
    int32_t send_value = value;
    BaseType_t tmp = pdPASS;

    if (!ss_fsm_eventqueue_get(name, &queue, &queue_prio)) SS_ERROR(NULL);

    if (prio){
        tmp = xQueueSend(queue_prio, &send_value, (TickType_t) 0);
    } else {
        tmp = xQueueSend(queue, &send_value, (TickType_t) 0);
    }

    if (tmp != pdPASS) {
        SS_ERROR("fsm event send failed");
    }

    return true;
}

bool ss_fsm_event_send(int32_t event) {
    char* name;

    if (!ss_fsm_event_get(RM_PRIO(event), &name)) SS_ERROR(NULL);

    return ss_fsm_event_send_to(name, RM_PRIO(event), HAS_PRIO(event));
}

bool ss_fsm_event_send_core(int32_t value) {
    return ss_fsm_event_send_to(SS_FSM_CORE, RM_PRIO(value), HAS_PRIO(value));
}


int32_t ss_fsm_event_receive_core() {
    return ss_fsm_event_receive(SS_FSM_CORE);
}

bool ss_fsm_event_get(int32_t event_code, char** task_name) {
    char *name = NULL;

    name = hmget(ss_fsm_event_hashmap, event_code);
    if (name == NULL) {
        SS_ERROR("unknown fsm event code");
    }

    *task_name = name;

    return true;
}

bool ss_fsm_event_add(int32_t event_code) {
    char* task_name = pcTaskGetName(NULL);

    hmput(ss_fsm_event_hashmap, event_code, task_name);

    return true;
}


#endif // COMPILE_SS_FSM
