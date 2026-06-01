#ifndef PWM_H
#define PWM_H

#include "main.h"
#include "stm32f4xx_ll_tim.h"

void PWM_Init(void);
void PWM_SetDuty(uint8_t channel, float duty);  // duty: 0.0 - 1.0

#endif
