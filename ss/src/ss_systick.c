#include <libopencm3/cm3/systick.h>
#include "ss_systick.h"
#include <inttypes.h>

extern Systick_Handle handle1;
extern Systick_Handle handle2;
extern Systick_Handle handle_pid;
extern Systick_Handle handle_error;
extern Systick_Handle handle_tod;

void sys_tick_handler(void) {
    handle1.tick++;
    handle2.tick++;
    handle_pid.tick++;
    handle_error.tick++;
    handle_tod.tick++;
}

int8_t ss_init_systick(uint32_t reload) {
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
    systick_set_reload(reload);
    systick_interrupt_enable();
    systick_counter_enable();
    return 0;
}

bool ss_handle_timer(Systick_Handle* handle) {
    if (handle->tick + handle->period < handle->timer) handle->timer = 0;
    if (handle->timer == 0) handle->timer = handle->tick + handle->period;
    if (handle->timer > handle->tick) return false;
    handle->timer = (handle->tick - handle->timer) > handle->period ? handle->tick + handle->period : handle->timer + handle->period;
    return true;
}