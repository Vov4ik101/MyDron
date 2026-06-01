// uart_hal.h
#ifndef UART_HAL_H
#define UART_HAL_H

#include "main.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart2;  // ваш UART

// Основные функции
void UART_SendByte(uint8_t data);
void UART_SendString(const char* str);
void UART_SendInt(int value);
void UART_SendFloat(float value);
void UART_SendHex(uint8_t value);
void UART_SendNewLine(void);
void print_array(uint8_t array[], uint8_t size);
uint8_t UART_Receive_Byte_Blocking(UART_HandleTypeDef *huart2);

#endif
