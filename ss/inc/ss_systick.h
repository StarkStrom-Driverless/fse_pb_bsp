#ifndef _SS_SYSTICK_H_
#define _SS_SYSTICK_H_


typedef struct Systick_Handle
{
    uint32_t timer;
    uint32_t period;
    volatile uint32_t tick;

} Systick_Handle;

void sys_tick_handler(void);

int8_t ss_init_systick(uint32_t reload);

bool ss_handle_timer(Systick_Handle* handle);

#endif