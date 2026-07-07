#include "loop.h"
#include "SPI_NRF.h"
#include "telemetry_packet.h"
#include <math.h>
extern volatile uint8_t EXTI_5; //Флаг, указывающий на то, что уровень на выводе IRQ у NRF переключился - пришел 32-байтный пакет данных
float roll_out;
float pitch_out;
float yaw_out;

static TelemetryPacket1 tx_packet1; // Выделение места для копирования принятого сообщения №1
static TelemetryPacket2 tx_packet2; // Выделение места для копирования принятого сообщения №2
static TelemetryPacket3 tx_packet3; // Выделение места для копирования принятого сообщения №3
static TelemetryPacket4 tx_packet4; // Выделение места для копирования принятого сообщения №4

float delta = 0.001f; //Переменная для изменения шага регулировки газа и PID
#define DEADBAND 0.001f   //Определи порог мёртвой зоны
float alpha = 0.1f;  // Коэффициент сглаживания для фильтрации показаний датчика (0.1 = сильное сглаживание, 0.9 = слабое)
static uint32_t last_time = 0; //Переменная для реализации задержки в коде
const uint32_t INTERVAL_MS = 200;  // Переменная для реализации задержки в коде (сколько ждать) в мс
const uint32_t INTERVAL_MS2 = 1;  // Вторая переменная для реализации задержки в коде (сколько ждать) в мс

void print_reg(void){ // Функция для вывода регистров NRF в USART. Применима для диагностики.
	UART_SendString ("STATUS: ");
	UART_SendHex(read_reg_NRF24(STATUS));

	UART_SendNewLine();
	UART_SendString("OBSERVE_TX: ");
	UART_SendHex(read_reg_NRF24(OBSERVE_TX));

	UART_SendNewLine();
	UART_SendString("FIFO_STATUS: ");
	UART_SendHex(read_reg_NRF24(FIFO_STATUS));
	UART_SendNewLine();
	UART_SendString("CONFIG: ");
	UART_SendHex(read_reg_NRF24(CONFIG));
	UART_SendNewLine();
}


void loop(void) {
uint8_t temp; // Вспомогательная переменная
uint8_t data[DATA_SIZE] = {0}; // Массив для приема данных по SPI
uint8_t data_out[DATA_SIZE] = {0}; // Массив для отправки данных по SPI
LL_SPI_Enable(SPI1); //Включаем передачу по SPI
init_NRF24L01(); // Процедура инициализации NRF
tx_default_addr(0x1111111111); // Задание адреса передачи
rx_default_addr(0x2222222222); // Задание адреса приема

typedef struct {
    float roll, pitch, yaw;
    float target_roll, target_pitch, target_yaw;
    float throttle;
    uint32_t last_tick;
} FlightData; // Структура для хранения данных полёта

static FlightData flight = {0};// Структура для хранения данных полёта, объявление
static PID pid_roll, pid_pitch, pid_yaw; // ПИД структуры соответственно для Roll, Pich, Yow
static float motor1, motor2, motor3, motor4; // Переменые для вывода PWM на моторы

MPU_Init(); // Инициализация MPU6050

// Проверка подключения
if (!MPU_TestConnection()) {
    while(1) {
    	UART_SendString("Error_not_find_MPU5060");  // Если датчик не откликается, то Ошибка — датчик не найден, шлем в USART
    }
}

PWM_Init(); // Инициализация PWM

PWM_SetDuty(1, 0.0f); // Сразу после инициализации отправляем "ноль газа"
PWM_SetDuty(2, 0.0f); // Сразу после инициализации отправляем "ноль газа"
PWM_SetDuty(3, 0.0f); // Сразу после инициализации отправляем "ноль газа"
PWM_SetDuty(4, 0.0f); // Сразу после инициализации отправляем "ноль газа"
HAL_Delay(5000);  // Даем ESC "увидеть" этот сигнал и выключить защиту
while(EXTI_5 == 0){};	//Ждем любой команды для запуска дрона

//PWM_SetDuty(1, 0.6f);
//PWM_SetDuty(2, 0.6f);
//PWM_SetDuty(3, 0.6f);
//PWM_SetDuty(4, 0.6f);
//while(EXTI_5 == 0){};
//HAL_Delay(20000);  // Ждем 5 секунды
//
//  PWM_SetDuty(1, 0.0f);
//  PWM_SetDuty(2, 0.0f);
//  PWM_SetDuty(3, 0.0f);
//  PWM_SetDuty(4, 0.0f);
//
//  HAL_Delay(20000);  // Ждем 5 секунды

PID_Init(&pid_roll, 0.015f,  0.00001f, 2.5f);
PID_Init(&pid_pitch, 0.010f,  0.00001f, 1.5f);
PID_Init(&pid_yaw, 0.0001f, 0.00f, 0.001f);

flight.target_roll = 0.0f; // Устанавливаем начальные значения целевых углов
flight.target_pitch = 0.1f; // Устанавливаем начальные значения целевых углов
flight.target_yaw = 0.0f; // Устанавливаем начальные значения целевых углов
flight.throttle = 0.37; // Устанавливаем начальное значение газа

    while(1) {
    	uint32_t now = HAL_GetTick(); // В переменную now записываем текущее время
        if (MPU_Update()) {
        	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_11); // Если пришли данные с датчика, то переключаем пин D11

        	// Применяем фильтр экспоненциальный к полученным сырым значениям с датчика
        	flight.roll = flight.roll * (1.0f - alpha) + MPU_GetRoll() * alpha;
        	flight.pitch = flight.pitch * (1.0f - alpha) + MPU_GetPitch() * alpha;
        	flight.yaw = flight.yaw * (1.0f - alpha) + MPU_GetYaw() * alpha;
        	// Узнаем отклонение от заданных величин для применения мертвой зоны
        	float roll_error = flight.target_roll - flight.roll;
        	float pitch_error = flight.target_pitch - flight.pitch;
        	float yaw_error = flight.target_yaw - flight.yaw;
        	// Применяем мёртвую зону
        	if (fabs(roll_error) < DEADBAND) flight.roll = 0.0f;
        	if (fabs(pitch_error) < DEADBAND) flight.pitch = 0.0f;
        	if (fabs(yaw_error) < DEADBAND) flight.yaw = 0.0f;
            // Получаем PID (от 0 до 1) для каждой составляющей полета
            roll_out = PID_Compute(&pid_roll, flight.target_roll, flight.roll, flight.throttle);
            pitch_out = PID_Compute(&pid_pitch, flight.target_pitch, flight.pitch, flight.throttle);
            yaw_out = PID_Compute(&pid_yaw, flight.target_yaw, flight.yaw, flight.throttle);

            //Применяем миксер для расчета уровня PWM, который надо подать на соответствующий мотор
            motor1 = flight.throttle + roll_out + pitch_out - yaw_out;
            motor2 = flight.throttle - roll_out + pitch_out + yaw_out;
            motor3 = flight.throttle + roll_out - pitch_out - yaw_out;
            motor4 = flight.throttle - roll_out - pitch_out + yaw_out;
            // Устанавливаем защитные ограничения
            motor1 = (motor1 > 1.0f) ? 1.0f : (motor1 < 0.0f) ? 0.0f : motor1;
            motor2 = (motor2 > 1.0f) ? 1.0f : (motor2 < 0.0f) ? 0.0f : motor2;
            motor3 = (motor3 > 1.0f) ? 1.0f : (motor3 < 0.0f) ? 0.0f : motor3;
            motor4 = (motor4 > 1.0f) ? 1.0f : (motor4 < 0.0f) ? 0.0f : motor4;

            // Отправляем на моторы PWM чигнал
            PWM_SetDuty(1, motor1);
            PWM_SetDuty(2, motor2);
            PWM_SetDuty(3, motor3);
            PWM_SetDuty(4, motor4);
            /*
            UART_SendString("roll_out:");
            UART_SendFloat(roll_out);
            UART_SendString("	");
            UART_SendString("pitch_out:");
            UART_SendFloat(pitch_out);
            UART_SendString("	");
            UART_SendString("yaw_out:");
            UART_SendFloat(yaw_out);
            UART_SendString("	");
            UART_SendString("throttle_out:");
            UART_SendFloat(flight.throttle);
            UART_SendString("	");
            UART_SendNewLine();
			*/
        }

        if(EXTI_5 == 1){
//        	rx_mode_NRF24();
        	read_FIFO(data);
        	//Регулируем throttle
        	if(data[0] == 1){
        		flight.throttle+=delta;
        	}else if(data[0] == 2){
        		flight.throttle-=delta;
        	}
        	//Регулируем RP
        	if(data[1] == 1){
        		pid_roll.kp+=delta;
        	}else if(data[1] == 2){
        		pid_roll.kp-=delta;
        	}
        	//Регулируем RI
        	if(data[2] == 1){
        		pid_roll.ki+=delta;
        	}else if(data[2] == 2){
        		pid_roll.ki-=delta;
        	}
        	//Регулируем RD
        	if(data[3] == 1){
        		pid_roll.kd+=delta;
        	}else if(data[3] == 2){
        		pid_roll.kd-=delta;
        	}
        	//Регулируем PP
        	if(data[4] == 1){
        		pid_pitch.kp+=delta;
        	}else if(data[4] == 2){
        		pid_pitch.kp-=delta;
        	}
        	//Регулируем PI
        	if(data[5] == 1){
        		pid_pitch.ki+=delta;
        	}else if(data[5] == 2){
        		pid_pitch.ki-=delta;
        	}
        	//Регулируем PD
        	if(data[6] == 1){
        		pid_pitch.kd+=delta;
        	}else if(data[6] == 2){
        		pid_pitch.kd-=delta;
        	}
        	//Регулируем YP
        	if(data[7] == 1){
        		pid_yaw.kp+=delta;
        	}else if(data[7] == 2){
        		pid_yaw.kp-=delta;
        	}
        	//Регулируем YI
        	if(data[8] == 1){
        		pid_yaw.ki+=delta;
        	}else if(data[8] == 2){
        		pid_yaw.ki-=delta;
        	}
        	//Регулируем YD
        	if(data[9] == 1){
        		pid_yaw.kd+=delta;
        	}else if(data[9] == 2){
        		pid_yaw.kd-=delta;
        	}
        	EXTI_5 = 0;						//Если пришло сообщение от NRF, то сбрасаываем флаг
        }


        if (now - last_time >= INTERVAL_MS) {
            //Заполняем стартовый байт и ид пакета в структуре1
            tx_packet1.start_byte = 0xAA;
            tx_packet1.packet_id = 5;
            //Заполняем roll, pich, yow пакета в структуре1
            tx_packet1.roll = roll_out;
            tx_packet1.pitch = pitch_out;
            tx_packet1.yaw = yaw_out;
            tx_packet1.throttle = flight.throttle;


            //Заполняем стартовый байт и ид пакета в структуре2
            tx_packet2.start_byte = 0xBB;
            tx_packet2.packet_id = 5;
            //Заполняем PID roll пакета в структуре
            tx_packet2.RP = pid_roll.kp;
            tx_packet2.RI = pid_roll.ki;
            tx_packet2.RD = pid_roll.kd;

            //Заполняем стартовый байт и ид пакета в структуре3
            tx_packet3.start_byte = 0xCC;
            tx_packet3.packet_id = 5;
            //Заполняем PID pich пакета в структуре3
            tx_packet3.PP = pid_pitch.kp;
            tx_packet3.PI = pid_pitch.ki;
            tx_packet3.PD = pid_pitch.kd;

            //Заполняем стартовый байт и ид пакета в структуре4
            tx_packet4.start_byte = 0xDD;
            tx_packet4.packet_id = 5;
            //Заполняем PID yow пакета в структуре4
            tx_packet4.YP = pid_yaw.kp;
            tx_packet4.YI = pid_yaw.ki;
            tx_packet4.YD = pid_yaw.kd;

            //Заполняем стартовый байт и ид пакета в структуре1
            tx_packet1.start_byte = 0xAA;
            tx_packet1.packet_id = 5;
            //Заполняем roll, pich, yow пакета в структуре1
            tx_packet1.roll = roll_out;
            tx_packet1.pitch = pitch_out;
            tx_packet1.yaw = yaw_out;
            tx_packet1.throttle = flight.throttle;
        	 last_time = now;

        	 if(EXTI_5 == 0){
        	 //Отправляем пакет структуры 1
        	 tx_packet1.size = sizeof(TelemetryPacket1);
        	 send_dataFlow_NRF24((uint8_t*)&tx_packet1, sizeof(TelemetryPacket1));

        	 //DWT_Delay(50);

        	 //Отправляем пакет структуры 2
        	 tx_packet2.size = sizeof(TelemetryPacket2);
        	 send_dataFlow_NRF24((uint8_t*)&tx_packet2, sizeof(TelemetryPacket2));

        	 //DWT_Delay(50);

        	 //Отправляем пакет структуры 3
        	 tx_packet3.size = sizeof(TelemetryPacket3);
        	 send_dataFlow_NRF24((uint8_t*)&tx_packet3, sizeof(TelemetryPacket3));

        	 //DWT_Delay(50);

        	//Отправляем пакет структуры 4
        	 tx_packet4.size = sizeof(TelemetryPacket4);
        	 send_dataFlow_NRF24((uint8_t*)&tx_packet4, sizeof(TelemetryPacket4));
        	 }
        	 //DWT_Delay(50);
        }
    	//HAL_Delay(100);  // ~100 Гц
    }
}



#define PUSK
#ifndef PUSK
void loop(void) {
	PWM_Init();

	    // 1. Сразу после инициализации отправляем строгий "ноль газа"
	    PWM_SetDuty(1, 0.0f);
	    PWM_SetDuty(2, 0.0f);
	    PWM_SetDuty(3, 0.0f);
	    PWM_SetDuty(4, 0.0f);

	    // 2. Даем ESC "увидеть" этот сигнал и включить защиту
	    HAL_Delay(5000);  // Ждем 2 секунды

	    // 3. Теперь подаем питание на ESC (подключаем батарею) и слышим звуки инициализации.

	    // 4. Плавно увеличиваем газ до 40%
	    UART_SendString("Starting motors...\r\n");
	    PWM_SetDuty(1, 0.4f);
	    PWM_SetDuty(2, 0.4f);
	    PWM_SetDuty(3, 0.4f);
	    PWM_SetDuty(4, 0.4f);
}
#endif
