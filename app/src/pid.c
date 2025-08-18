#include "pid.h"

#include <assert.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/time_units.h>

LOG_MODULE_REGISTER(pid);

static float pid_constrain(const float val, const pid_limit_t* limit)
{
    if (val > limit->limit_high)
    {
        return limit->limit_high;
    }
    if (val < limit->limit_low)
    {
        return limit->limit_low;
    }
    return val;
}

int pid_init(pid_t* pid, const float Kp, const float Ki, const float Kd, const float lim_low, const float lim_high,
             const float int_lim_low, const float int_lim_high)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;

    pid->last_time = 0;
    pid->last_command = 0;
    pid->last_error = 0;
    pid->proportional_error = 0;
    pid->integral_error = 0;
    pid->derivative_error = 0;
    pid->set_point = 100;

    pid->output_limit.limit_low = lim_low;
    pid->output_limit.limit_high = lim_high;

    pid->integral_limit.limit_low = int_lim_low;
    pid->integral_limit.limit_high = int_lim_high;

    return 0;
}

int pid_set_gains(pid_t* pid, float Kp, float Ki, float Kd)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;

    return 0;
}

int pid_set_setpoint(pid_t* pid, const float set_point)
{
    pid->set_point = set_point;
    return 0;
}

int pid_set_time_function(pid_t* pid, uint64_t (*timer_func)(void))
{
    pid->get_time = timer_func;
    return 0;
}

float pid_process(pid_t* pid, const float temp)
{
    float command = 0;
    __ASSERT(getime != NULL, "getime function pointer is null");

    float actual_time = k_cyc_to_us_floor32(pid->get_time());
    float time_diff = (actual_time - pid->last_time) / 1000000.0f; // Convert to seconds
    float error = pid->set_point - temp;

    pid->proportional_error = pid->Kp * error;
    pid->integral_error += pid->Ki * (error + pid->last_error) * 0.5f * time_diff;
    pid->derivative_error = pid->Kd * (error - pid->last_error) / time_diff;

    pid->integral_error = pid_constrain(pid->integral_error, &pid->integral_limit);

    command = pid->proportional_error + pid->integral_error + pid->derivative_error;

    pid->last_time = actual_time;
    pid->last_error = error;

    command = pid_constrain(command, &pid->output_limit);

    return command;
}
