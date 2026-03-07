#include "ss_config.h"

#if COMPILE_SS_PID

#ifndef _SS_PID_H_
#define _SS_PID_H_

struct SS_PID {

	/* Controller gains */
	float ss_pid_kp;
	float ss_pid_ki;
	float ss_pid_kd;

	/* Derivative low-pass filter time constant */
	float ss_pid_tau;

	/* Output limits */
	float ss_pid_out_min;
	float ss_pid_out_max;

	/* Integrator limits */
	float ss_pid_integrator_min;
	float ss_pid_integrator_max;

	/* Sample time (in seconds) */
	float ss_pid_period;

	/* Controller "memory" */
	float ss_pid_int;
	float ss_pid_error_prev;			/* Required for ss_pid_int */
	float ss_pid_dif;
	float ss_pid_measurment_prev;		/* Required for ss_pid_dif */

	/* Controller output */
	float ss_pid_out;

	float ss_pid_setpoint;

};

void ss_pid_init(struct SS_PID* pid);

void ss_pid_update(struct SS_PID* pid, float setpoint, float measurment, float* output);

#endif // _SS_PID_H_

#endif // COMPILE_SS_PID
