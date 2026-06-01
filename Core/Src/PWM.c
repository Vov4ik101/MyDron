#include "PWM.h"


#define PWM_MIN 1000      // 1 мс (минимальный газ)
#define PWM_MAX 2000      // 2 мс (максимальный газ)

void PWM_Init(void) {

    // 1. Включение выходов каналов
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH2);
    LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH3);
    LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH4);

    // 2. Запуск таймеров
    LL_TIM_EnableCounter(TIM3);
    LL_TIM_EnableCounter(TIM4);

    // 8. Включение основных выходов (для продвинутых таймеров)
    LL_TIM_EnableAllOutputs(TIM3);
    LL_TIM_EnableAllOutputs(TIM4);
}

void PWM_SetDuty(uint8_t channel, float duty) {
    // Ограничиваем duty от 0.0 до 1.0
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;

    // Вычисляем длительность импульса
    uint32_t pulse = PWM_MIN + (uint32_t)(duty * (PWM_MAX - PWM_MIN));

    // Защитное ограничение
    if (pulse > PWM_MAX) pulse = PWM_MAX;
    if (pulse < PWM_MIN) pulse = PWM_MIN;

    // Устанавливаем Compare-регистр для соответствующего канала
    switch(channel) {
        case 1:
            LL_TIM_OC_SetCompareCH1(TIM3, pulse);
            break;
        case 2:
            LL_TIM_OC_SetCompareCH2(TIM3, pulse);
            break;
        case 3:
            LL_TIM_OC_SetCompareCH3(TIM4, pulse);
            break;
        case 4:
            LL_TIM_OC_SetCompareCH4(TIM4, pulse);
            break;
        default:
            // Неверный канал
            break;
    }
}
