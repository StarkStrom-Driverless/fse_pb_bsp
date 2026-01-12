#include "ss_fsm.h"

#ifndef typeof
#define typeof __typeof__
#endif

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include <stdbool.h>

#define SS_FEEDBACK_BASE SS_FEEDBACK_BASE_NOT_SET

SS_FSM_EVENTQUEUE_HASHMAP_t* ss_fsm_eventqueue_hashmap = NULL;
SS_FSM_EVENT_HASHMAP_t* ss_fsm_event_hashmap = NULL;

SS_FEEDBACK ss_fsm_eventqueue_add(char* name) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    QueueHandle_t queue = NULL;
    static bool init_core = false;
    
    if (init_core == false) {
        init_core = true;
        rc = ss_fsm_eventqueue_add(SS_FSM_CORE);
        SS_HANDLE_ERROR_WITH_EXIT(rc);
    }

    queue = xQueueCreate(3, sizeof(uint8_t));
    if (queue == NULL) {
        rc = SS_FEEDBACK_FSM_INIT_ERROR;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    shput(ss_fsm_eventqueue_hashmap, name, queue);

    return rc;
}

SS_FEEDBACK ss_fsm_eventqueue_get(char* name, QueueHandle_t* handle) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    QueueHandle_t queue;

    queue = shget(ss_fsm_eventqueue_hashmap, name);
    if (queue == NULL) {
        rc = SS_FEEDBACK_FSM_WRONG_KEY;
    }
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    *handle = queue;

    return rc;
}

SS_FEEDBACK ss_fsm_event_receive(char *name, uint8_t* value) {
    SS_FEEDBACK rc = SS_FEEDBACK_FSM_RECEIVED_EVENT;
    QueueHandle_t queue;

    rc = ss_fsm_eventqueue_get(name, &queue);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    BaseType_t tmp = xQueueReceive(queue, value, ( TickType_t ) 10 ); 
    if (tmp != pdPASS) {
        rc = SS_FEEDBACK_FSM_NOT_RECEIVED_EVENT;
    } else {
        rc = SS_FEEDBACK_FSM_RECEIVED_EVENT;
    }

    return rc;
}

SS_FEEDBACK ss_fsm_event_send_to(char *name, uint8_t value) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    QueueHandle_t queue;
    uint8_t send_value = value;

    rc = ss_fsm_eventqueue_get(name, &queue);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    BaseType_t tmp = xQueueSend(queue, &send_value, ( TickType_t ) 10);
    if (tmp != pdPASS) {
        rc = SS_FEEDBACK_FSM_EVENT_SEND_FAILED;
    }

    return rc;
}

SS_FEEDBACK ss_fsm_event_send(uint8_t event) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    char* name;

    rc = ss_fsm_event_get(event, &name);
    SS_HANDLE_ERROR_WITH_EXIT(rc);

    rc = ss_fsm_event_send_to(name, event);

    return rc;
}

SS_FEEDBACK ss_fsm_event_send_core(uint8_t value) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    rc = ss_fsm_event_send_to(SS_FSM_CORE, value);
    return rc;
}


SS_FEEDBACK ss_fsm_event_receive_core(uint8_t* value) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;
    rc = ss_fsm_event_receive(SS_FSM_CORE, value);
    return rc;
}

SS_FEEDBACK ss_fsm_event_get(uint8_t event_code, char** task_name) {
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

SS_FEEDBACK ss_fsm_event_add(uint8_t event_code, char* task_name) {
    SS_FEEDBACK rc = SS_FEEDBACK_OK;

    hmput(ss_fsm_event_hashmap, event_code, task_name);

    return rc;
}

