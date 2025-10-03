
#ifndef PID_H
#define PID_H

//- rsb: Tuning PID:
// https://www.zhinst.com/ch/en/resources/principles-of-pid-controllers

//- rsb: kp,ki,kd - unit gains per unit output per unit step
typedef struct PID_Gains PID_Gains;
struct PID_Gains
{
	r32 kp;
	r32 ki;
	r32 kd;
};

typedef struct PID PID;
struct PID
{
    //- rsb: state
	r32 target;
	r32 actual;
	r32 output;
	r32 scaled_output;
	r32 prev_error;
	r32 error_sum;
	
	//- rsb: debug
	r32 error;
	r32 P;
	r32 I;
	r32 D;
	
	//- rsb: settings
	PID_Gains gains;
	r32 max_output;
	Rng2F32 error_sum_limit;
};

//
//- rsb: inits
//

#define pid_gains(p,i,d,dt) &(PID_Gains){.kp=(p), .ki=(i)*dt, .kd=(d)/dt}
internal PID_Gains *
pid_gains_init(PID_Gains *gains, r32 kp, r32 ki, r32 kd, r32 step_dt)
{
	gains->kp = kp;
	gains->ki = ki * step_dt; // pre multiply dt to reduce addtional mul in pid_step
	gains->kd = kd / step_dt; // pre divide dt to reduce addtional div in pid_step
	return gains;
}

//- rsb: zn - ziegler_nichols
#define pid_zn_gains(ku,hz,dt) pid_gains(0.6f*ku, 1.2f*ku*hz, 0.075*ku/hz, dt)
internal PID_Gains *
pid_zn_gains_init(PID_Gains *gains, r32 ku, r32 stable_oscillation_hz, r32 step_dt)
{
	r32 tu = 1.0f / stable_oscillation_hz;
	PID_Gains *result = pid_gains_init(gains, 0.6f * ku, 1.2f * ku / tu, 0.075f * ku * tu, step_dt);
	return result;
}

internal void
pid_update_gains(PID *pid, PID_Gains *gains)
{
	MemoryCopyStruct(&pid->gains, gains);
}

internal void
pid_init(PID *pid, PID_Gains *gains, r32 min_error_sum, r32 max_error_sum, r32 max_output)
{
	MemoryZeroStruct(pid);
	pid_update_gains(pid, gains);
	pid->max_output = max_output;
	pid->error_sum_limit = rng2f32(min_error_sum / pid->gains.ki, max_error_sum / pid->gains.ki);
}

//
//- rsb: helpers
//

internal void
pid_reset(PID *pid)
{
	pid->prev_error = 0.0f;
	pid->error_sum = 0.0f;
}
r32 output_pid;
r32 output_pid_clamp;
internal inline r32 
pid_step(PID *pid, r32 target, r32 actual)
{
	pid->target = target;
	pid->actual = actual;
	pid->error = target - actual;
	pid->error_sum += pid->error;
	pid->error_sum = clamp_rng2f32(pid->error_sum_limit, pid->error_sum);
	pid->P = pid->gains.kp * pid->error;
	pid->I = pid->gains.ki * pid->error_sum;
	pid->D = pid->gains.kd * (pid->error - pid->prev_error);
	pid->output = pid->P + pid->I + pid->D;
    output_pid_clamp = pid->output;
	pid->output = Clamp(-1.0f, pid->output, 1.0f);
    output_pid = pid->output;
	pid->scaled_output = pid->output * pid->max_output;
	pid->prev_error = pid->error;
	return pid->scaled_output;
}

#endif //PID_H
