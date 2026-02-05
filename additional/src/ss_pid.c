#include "ss_pid.h"

void ss_pid_init(struct SS_PID* pid) {
	/* Clear controller variables */
	pid->ss_pid_int = 0.0f;
	pid->ss_pid_error_prev  = 0.0f;

	pid->ss_pid_dif  = 0.0f;
	pid->ss_pid_measurment_prev = 0.0f;

	pid->ss_pid_out = 0.0f;

	pid->ss_pid_setpoint = 0.0f;
}


void ss_pid_update(struct SS_PID* pid, float input, float* output) {


	/*
	* Error signal
	*/
    float error = pid->ss_pid_setpoint - input;


	/*
	* Proportional
	*/
    float proportional = pid->ss_pid_kp * error;


	/*
	* Integral
	*/
    pid->ss_pid_int = pid->ss_pid_int + 0.5f * pid->ss_pid_ki * pid->ss_pid_period * (error + pid->ss_pid_error_prev);

	/* Anti-wind-up via ss_pid_int clamping */
	
    if (pid->ss_pid_int > pid->ss_pid_integrator_max) {

        pid->ss_pid_int = pid->ss_pid_integrator_max;

    } else if (pid->ss_pid_int < pid->ss_pid_integrator_min) {

        pid->ss_pid_int = pid->ss_pid_integrator_min;
    }
	
	


	/*
	* Derivative (band-limited ss_pid_dif)
	*/
		
    pid->ss_pid_dif = -(2.0f * pid->ss_pid_kd * (input - pid->ss_pid_measurment_prev)	/* Note: derivative on measurement, therefore minus sign in front of equation! */
                        
						
						+ (2.0f * pid->ss_pid_tau - pid->ss_pid_period) * pid->ss_pid_dif)
                        / (2.0f * pid->ss_pid_tau + pid->ss_pid_period);


	/*
	* Compute output and apply limits
	*/
	pid->ss_pid_out = proportional + pid->ss_pid_int + pid->ss_pid_dif;

    if (pid->ss_pid_out > pid->ss_pid_out_max) {

        pid->ss_pid_out = pid->ss_pid_out_max;

    } else if (pid->ss_pid_out < pid->ss_pid_out_min) {

        pid->ss_pid_out = pid->ss_pid_out_min;

    }

	/* Store error and measurement for later use */
    pid->ss_pid_error_prev       = error;
    pid->ss_pid_measurment_prev = input;

	/* Return controller output */
    *output = pid->ss_pid_out;
}