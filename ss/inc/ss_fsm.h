#ifndef _SS_FSM_H_
#define _SS_FSM_H_

#include "FreeRTOS.h"
#include <queue.h>

#include "ss_feedback.h"

#define SS_FSM_CORE "SS_FSM_CORE"

struct SS_FSM_EVENTQUEUE_HASHMAP {
    char* key;
    QueueHandle_t value;
};
typedef struct SS_FSM_EVENTQUEUE_HASHMAP SS_FSM_EVENTQUEUE_HASHMAP_t;


struct SS_FSM_EVENT_HASHMAP {
    uint8_t key;
    char* value;
};
typedef struct SS_FSM_EVENT_HASHMAP SS_FSM_EVENT_HASHMAP_t;


extern SS_FSM_EVENTQUEUE_HASHMAP_t* ss_fsm_eventqueue_hashmap;
extern SS_FSM_EVENT_HASHMAP_t* ss_fsm_event_hashmap;

SS_FEEDBACK ss_fsm_eventqueue_add(char* name);
SS_FEEDBACK ss_fsm_eventqueue_get(char* name, QueueHandle_t* handle);
SS_FEEDBACK ss_fsm_event_receive(char *name, uint8_t* value);
SS_FEEDBACK ss_fsm_event_send_to(char *name, uint8_t value);
SS_FEEDBACK ss_fsm_event_send_core(uint8_t value);
SS_FEEDBACK ss_fsm_event_receive_core(uint8_t* value);
SS_FEEDBACK ss_fsm_event_get(uint8_t event_code, char** task_name);
SS_FEEDBACK ss_fsm_event_add(uint8_t event_code, char* task_name);
SS_FEEDBACK ss_fsm_event_send(uint8_t event);

#endif