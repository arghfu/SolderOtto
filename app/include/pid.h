#pragma once

// #define KP_T210         196
// #define KI_T210         38
// #define KD_T210         172.8
// #define MAX_I_T210      300

#define KP_T210 		7
#define KI_T210 		4
#define KD_T210 		0.3
#define MAX_I_T210 		300

#define KP_T245         8
#define KI_T245         2
#define KD_T245         0.5
#define MAX_I_T245      300
#include <stddef.h>
#include <stdint.h>

typedef struct pid_limit
{
    float limit_low;
    float limit_high;
} pid_limit_t;

typedef struct pid
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
    pid_limit_t output_limit;
    pid_limit_t integral_limit;
    uint64_t (*get_time)(void);
} pid_t;

int pid_init(pid_t* pid, float Kp, float Ki, float Kd, float lim_low, float lim_high,
             float int_lim_low, float int_lim_high);
int pid_set_gains(pid_t* pid, float Kp, float Ki, float Kd);
int pid_set_setpoint(pid_t* pid, float set_point);
float pid_process(pid_t* pid, float temp);
int pid_set_time_function(pid_t* pid, uint64_t (*timer_func)(void));
