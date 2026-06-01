// pid.h
#ifndef PID_H
#define PID_H

#include "main.h"

typedef struct {
    float kp, ki, kd;
    float integral;
    float prev_error;
    float output_limit;
} PID;

void PID_Init(PID* pid, float kp, float ki, float kd);
float PID_Compute(PID* pid, float target, float current);

#endif
