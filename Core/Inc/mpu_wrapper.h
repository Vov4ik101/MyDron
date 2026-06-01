#ifndef MPU_WRAPPER_H
#define MPU_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// Инициализация MPU6050
void MPU_Init(void);

// Обновление данных (вызывать в цикле)
bool MPU_Update(void);

// Получение углов (после MPU_Update)
float MPU_GetRoll(void);
float MPU_GetPitch(void);
float MPU_GetYaw(void);

// Проверка подключения
bool MPU_TestConnection(void);

#ifdef __cplusplus
}
#endif

#endif
