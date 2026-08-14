#include "ss.h"

uint16_t led = PIN('C', 1);

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

/* must match FixedStep in fse_pb_bsp/simulink/ss_model_config.m (0.001 s) */
#define SIMULINK_STEP_MS        1

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
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SIMULINK_STEP_MS));
    }
}

#endif /* SS_SIMULINK_MODEL */

static void blinky_task(void *args) {
    uint8_t value = 0;

    for (;;) {
        ss_io_write(led, value);

        value = (value == SS_GPIO_ON) ? SS_GPIO_OFF : SS_GPIO_ON;

        ss_rtos_delay_ms(500);
    }
}

int main(void)
{
    SS_ERROR_ASSERT(ss_init());

    SS_ERROR_ASSERT(ss_io_init(led, SS_GPIO_MODE_OUTPUT));

    SS_ERROR_ASSERT(ss_rtos_task_add(blinky_task, NULL, 1, "blinky_task"));

#ifdef SS_SIMULINK_MODEL
    /* above blinky: the model is the control loop and must not be delayed by
       housekeeping. ss_rtos_task_add gives it 1024 words - swap in
       ss_rtos_big_task_add (2048) if a growing model overflows the stack,
       configCHECK_FOR_STACK_OVERFLOW is on and will land in the hook below */
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
