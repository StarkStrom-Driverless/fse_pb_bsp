#include <libopencm3/cm3/systick.h>
#include "ss_systick.h"
#include <inttypes.h>

extern Systick_Handle handle1;
extern Systick_Handle handle2;
extern Systick_Handle handle_pid;
extern Systick_Handle handle_error;
extern Systick_Handle handle_tod;

extern struct SystickCntr systick_cntr;

#ifndef SS_USE_RTOS
void sys_tick_handler(void) {
    for (uint8_t i = 0; i < systick_cntr.count; i++) {
        systick_cntr.handler[i].tick++;
    }
}
#endif

int8_t ss_init_systick(uint32_t reload) {
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
    systick_set_reload(reload);
    systick_interrupt_enable();
    systick_counter_enable();
    return 0;
}


uint8_t ss_handle_timer(Systick_Handle* handle) {
    if (handle->tick + handle->period < handle->timer) handle->timer = 0;
    if (handle->timer == 0) handle->timer = handle->tick + handle->period;
    if (handle->timer > handle->tick) return false;
    handle->timer = (handle->tick - handle->timer) > handle->period ? handle->tick + handle->period : handle->timer + handle->period;
    return true;
}

void ss_init_systick_cntr(struct SystickCntr* cntr) {
    cntr->count = 0;
}

uint8_t ss_systick_add_handle(struct SystickCntr* cntr, uint32_t period) {
    cntr->handler[cntr->count].timer = 0;
    cntr->handler[cntr->count].period = period;
    cntr->handler[cntr->count].tick = 0;
    
    return cntr->count++;
}

uint8_t ss_systick_expired(struct SystickCntr* cntr, uint8_t id) {
    return ss_handle_timer(&cntr->handler[id]);
}