#include "ss_config.h"

#if COMPILE_SS_FSM

#ifndef _SS_FSM_H_
#define _SS_FSM_H_

#include "FreeRTOS.h"
#include <queue.h>
#include "stdbool.h"

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

#define PRIO(v) v | (1 << 30)
#define HAS_PRIO(v) ((v & (1 << 30))?true:false)
#define RM_PRIO(v) (v & ~(1 << 30))


extern SS_FSM_EVENTQUEUE_HASHMAP_t* ss_fsm_eventqueue_hashmap;
extern SS_FSM_EVENTQUEUE_HASHMAP_t* ss_fsm_eventqueue_prio_hashmap;
extern SS_FSM_EVENT_HASHMAP_t* ss_fsm_event_hashmap;

#ifdef USE_PRIVATE
bool ss_fsm_eventqueue_add(char* name);
#endif
#ifdef USE_PRIVATE
bool ss_fsm_eventqueue_get(char* name, QueueHandle_t* handle, QueueHandle_t* handle_prio);
#endif

int32_t ss_fsm_event_receive(char *name);

#ifdef USE_PRIVATE
bool ss_fsm_event_send_to(char *name, int32_t value, bool prio);
#endif

bool ss_fsm_event_send_core(int32_t value);

int32_t ss_fsm_event_receive_core();

#ifdef USE_PRIVATE
bool ss_fsm_event_get(int32_t event_code, char** task_name);
#endif

bool ss_fsm_event_add(int32_t event_code);

bool ss_fsm_event_send(int32_t event);

#endif // _SS_FSM_H_

#endif // COMPILE_SS_FSM
