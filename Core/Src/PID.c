// pid.c
#include "pid.h"

void PID_Init(PID* pid, float kp, float ki, float kd) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output_limit = 1.0f;
}

float PID_Compute(PID* pid, float target, float current) {
    float error = target - current;

    // ѕропорциональна€ составл€юща€
    float P = pid->kp * error;

    // »нтегральна€ составл€юща€ (с ограничением насыщени€)
    pid->integral += error;
    if (pid->integral > pid->output_limit) pid->integral = pid->output_limit;
    if (pid->integral < -pid->output_limit) pid->integral = -pid->output_limit;
    float I = pid->ki * pid->integral;

    // ƒифференциальна€ составл€юща€
    float D = pid->kd * (error - pid->prev_error);
    pid->prev_error = error;

    float output = P + I + D;

    // ќграничение выхода
    if (output > pid->output_limit) output = pid->output_limit;
    if (output < -pid->output_limit) output = -pid->output_limit;

    return output;
}
