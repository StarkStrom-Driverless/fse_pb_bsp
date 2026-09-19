#include "ss.h"

/*
 * Simulink model task.
 *
 * SS_SIMULINK_MODEL is defined by the Makefile as soon as
 * usr/simulink/<model>_ert_rtw exists, so everything below disappears until
 * ./ss matlab_build has generated code - a fresh checkout still builds.
 *
 * ss_model_config.m selects the ert target with GenerateSampleERTMain off and
 * CodeInterfacePackaging 'Nonreusable function', so the generated interface is
 * <model>_initialize() plus <model>_step(). IncludeMdlTerminateFcn is off,
 * there is deliberately no terminate function to call.
 */
#ifdef SS_SIMULINK_MODEL

#define SS_STR_(x)      #x
#define SS_STR(x)       SS_STR_(x)
#define SS_CAT_(a, b)   a##b
#define SS_CAT(a, b)    SS_CAT_(a, b)

#include SS_STR(SS_SIMULINK_MODEL.h)

#define ss_model_initialize     SS_CAT(SS_SIMULINK_MODEL, _initialize)
#define ss_model_step           SS_CAT(SS_SIMULINK_MODEL, _step)

static void simulink_task(void *args) {
    TickType_t last_wake = xTaskGetTickCount();

    /* runs here rather than in main(): the generated initialize calls the
       StartFcnSpec of every block, e.g. ss_io_init, so it has to happen after
       ss_init() and it is simplest to keep it next to the loop it belongs to */
    ss_model_initialize();

    for (;;) {
        ss_model_step();

        /* absolute delay - with ss_rtos_delay_ms the period would drift by
           however long the step took, which a fixed step model cannot afford */
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SS_MODEL_STEP_MS));
    }
}

#endif /* SS_SIMULINK_MODEL */

int main(void)
{
    SS_ERROR_ASSERT(ss_init());

#ifdef SS_SIMULINK_MODEL
    SS_ERROR_ASSERT(ss_rtos_task_add(simulink_task, NULL, 2, "simulink_task"));
#endif

    ss_rtos_start();

    while (1) {

	}

    return 0;

}

void ss_error_fail(void) {
    while (1) {
        ss_led_error_toggle();
        ss_delay(1000);
    }
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName) {
    while(1) {
        ss_led_error_toggle();
        ss_delay(500);
    }
}
