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

float PID_Compute(PID* pid, float target, float current, float throttle) {
    // 1. Ошибка
    float error = target - current;

    // 2. Проверка на бесконечность
    if (isinf(error) || isnan(error)) {
        error = 0.0f;
    }

    // 3. Динамическая коррекция коэффициентов в зависимости от газа
    //    При увеличении газа P и D уменьшаются, чтобы избежать колебаний
    float throttle_factor = 1.0f;// + (throttle - 0.3f) * 0.5f;
    if (throttle_factor < 0.7f) throttle_factor = 0.7f;
    if (throttle_factor > 1.5f) throttle_factor = 1.5f;

    float kp_cur = pid->kp / throttle_factor;
    float kd_cur = pid->kd / throttle_factor;

    // 4. Пропорциональная составляющая
    float P = kp_cur * error;

    // 5. Интегральная составляющая (с анти-виндапом)
    pid->integral += error;

    if (pid->integral > pid->output_limit) {
        pid->integral = pid->output_limit;
    } else if (pid->integral < -pid->output_limit) {
        pid->integral = -pid->output_limit;
    }

    float I = pid->ki * pid->integral;

    // 6. Дифференциальная составляющая
    float D = kd_cur * (error - pid->prev_error);
    if (isinf(D) || isnan(D)) D = 0.0f;

    pid->prev_error = error;

    // 7. Сумма
    float output = P + I + D;

    // 8. Ограничение выхода
    if (output > pid->output_limit) {
        output = pid->output_limit;
    } else if (output < -pid->output_limit) {
        output = -pid->output_limit;
    }

    return output;
}
