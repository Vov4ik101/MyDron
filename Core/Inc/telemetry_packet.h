// telemetry_packet.h
#ifndef TELEMETRY_PACKET_H
#define TELEMETRY_PACKET_H

#include <stdint.h>


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

} TelemetryPacket;
#pragma pack(pop)

#endif
