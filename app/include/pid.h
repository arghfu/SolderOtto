#ifndef PID_H
#define PID_H

#define KP_T210 		7
#define KI_T210 		4
#define KD_T210 		0.3
#define MAX_I_T210 		300

#define KP_T245         8
#define KI_T245         2
#define KD_T245         0.5
#define MAX_I_T245      300

#include <stdint.h>

struct pid_limit
{
    float limit_low;
    float limit_high;
};

struct pid_data
{
    float Kp;
    float Ki;
    float Kd;
};

struct pid
{
    float Kp;
    float Ki;
    float Kd;
    float last_time;
    float last_error;
    float last_command;
    float proportional_error;
    float integral_error;
    float derivative_error;
    float set_point;
    struct pid_limit output_limit;
    struct pid_limit integral_limit;
    uint64_t (*get_time)(void);
};

int pid_init(struct pid* pid, float Kp, float Ki, float Kd, float lim_low, float lim_high,
             float int_lim_low, float int_lim_high);
int pid_set_gains(struct pid* pid, float Kp, float Ki, float Kd);
int pid_set_setpoint(struct pid* pid, float set_point);
float pid_process(struct pid* pid, float temp);
int pid_set_time_function(struct pid* pid, uint64_t (*timer_func)(void));
void pid_load_settings(struct pid* pid, struct pid_data* data);
#endif // PID_H