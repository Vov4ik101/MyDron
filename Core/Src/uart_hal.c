// uart_hal.c
#include "uart_hal.h"

void UART_SendByte(uint8_t data) {
    HAL_UART_Transmit(&huart2, &data, 1, 100);
}

void UART_SendString(const char* str) {
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 100);
}

void UART_SendInt(int value) {
    char buffer[16];
    sprintf(buffer, "%d", value);
    UART_SendString(buffer);
}

void UART_SendFloat(float value) {
    char buffer[32];
    sprintf(buffer, "%.2f", value);
    UART_SendString(buffer);
}

void UART_SendHex(uint8_t value) {
    char buffer[8];
    sprintf(buffer, "0x%02X", value);
    UART_SendString(buffer);
}

void UART_SendNewLine(void) {
    UART_SendString("\r\n");
}

void print_array(uint8_t array[], uint8_t size){									// Печать массива в шестнадцетиричных числах
	for (uint8_t i=0; i<size; i++){
		UART_SendHex(array[i]);
		UART_SendString(" ");
	}
}

uint8_t UART_Receive_Byte_Blocking(UART_HandleTypeDef *huart2) {
    uint8_t data;

    // HAL_MAX_DELAY означает "ждать вечно"
    HAL_UART_Receive(huart2, &data, 1, HAL_MAX_DELAY);

    return data;
}
