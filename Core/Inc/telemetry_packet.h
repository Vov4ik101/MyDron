// telemetry_packet.h
#ifndef TELEMETRY_PACKET_H
#define TELEMETRY_PACKET_H

#include <stdint.h>

//---------------------------------------------------------------------------------
//Пакет телеметрии 1
#pragma pack(push, 1)  // Выравнивание по 1 байту для точной передачи
typedef struct {
    // Заголовок пакета (для синхронизации)
    uint8_t start_byte;      // 0xAA (маркер начала пакета)
    uint8_t size;
    uint8_t packet_id;       // ID пакета (для отслеживания потерь)

    // Данные с дрона (телеметрия)
    float roll;              // крен (градусы)
    float pitch;             // тангаж (градусы)
    float yaw;               // рыскание (градусы)
    float throttle;          // газ (0.0 - 1.0)

} TelemetryPacket1;
#pragma pack(pop)
//---------------------------------------------------------------------------------
//Пакет телеметрии 2
#pragma pack(push, 1)  // Выравнивание по 1 байту для точной передачи
typedef struct {
    // Заголовок пакета (для синхронизации)
    uint8_t start_byte;      // 0xAA (маркер начала пакета)
    uint8_t size;
    uint8_t packet_id;       // ID пакета (для отслеживания потерь)

    // Данные с дрона (телеметрия)
    float RP;				//Roll P
    float RI;				//Roll I
    float RD;				//Roll D

} TelemetryPacket2;
#pragma pack(pop)
//---------------------------------------------------------------------------------
//Пакет телеметрии 3
#pragma pack(push, 1)  // Выравнивание по 1 байту для точной передачи
typedef struct {
    // Заголовок пакета (для синхронизации)
    uint8_t start_byte;      // 0xAA (маркер начала пакета)
    uint8_t size;
    uint8_t packet_id;       // ID пакета (для отслеживания потерь)

    // Данные с дрона (телеметрия)
    float PP;				//Pich P
    float PI;				//Pich I
    float PD;				//Pich D

} TelemetryPacket3;
#pragma pack(pop)
//---------------------------------------------------------------------------------
//Пакет телеметрии 4
#pragma pack(push, 1)  // Выравнивание по 1 байту для точной передачи
typedef struct {
    // Заголовок пакета (для синхронизации)
    uint8_t start_byte;      // 0xAA (маркер начала пакета)
    uint8_t size;
    uint8_t packet_id;       // ID пакета (для отслеживания потерь)

    // Данные с дрона (телеметрия)
    float YP;				//Yow P
    float YI;				//Yow I
    float YD;				//Yow D

} TelemetryPacket4;
#pragma pack(pop)

#endif
