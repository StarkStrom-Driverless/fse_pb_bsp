#ifndef _SS_SYSTICK_H_
#define _SS_SYSTICK_H_

#define MAX_SYSTICK_HANDLE_COUNT 10


typedef struct Systick_Handle
{
    uint32_t timer;
    uint32_t period;
    volatile uint32_t tick;

} Systick_Handle;

struct SystickCntr {
    Systick_Handle handler[MAX_SYSTICK_HANDLE_COUNT];
    uint8_t count;
};

void ss_init_systick_cntr(struct SystickCntr* cntr);

uint8_t ss_systick_add_handle(struct SystickCntr* cntr, uint32_t period);

bool ss_systick_expired(struct SystickCntr* cntr, uint8_t id);

void sys_tick_handler(void);

int8_t ss_init_systick(uint32_t reload);

bool ss_handle_timer(Systick_Handle* handle);

#endif