#include "loop.h"
#include "SPI_NRF.h"
#include "telemetry_packet.h"
extern volatile uint8_t EXTI_5;
static uint8_t packet_counter = 1;
static TelemetryPacket tx_packet;

void print_reg(void){
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
	LL_SPI_Enable(SPI1);//Запуск SPI1

	 uint8_t temp;																	// Вспомогательная переменная
	 uint8_t data[DATA_SIZE] = {0};													// Массив для обмена данными
	 uint8_t data_out[DATA_SIZE] = {0};												// Массив для отправки коэффициентов
	init_NRF24L01();
	tx_default_addr(0x1111111111);													// Задание адреса передачи
	rx_default_addr(0x2222222222);													// Задание адреса приема

    // Структура для хранения данных полёта
    typedef struct {
        float roll, pitch, yaw;
        float target_roll, target_pitch, target_yaw;
        float throttle;
        uint32_t last_tick;
    } FlightData;

    static FlightData flight = {0};
    static PID pid_roll, pid_pitch, pid_yaw;
    static float motor1, motor2, motor3, motor4;

    // Инициализация MPU6050
    MPU_Init();

    // Проверка подключения
    if (!MPU_TestConnection()) {
        while(1) {
        	UART_SendString("Error_not_find_MPU5060");  // Ошибка — датчик не найден
        }
    }

    // Инициализация PWM
    PWM_Init();

    // Настройка PID
    PID_Init(&pid_roll, 2.5f, 0.05f, 1.2f);
    PID_Init(&pid_pitch, 2.5f, 0.05f, 1.2f);
    PID_Init(&pid_yaw, 1.5f, 0.02f, 0.8f);

    // Начальные значения
    flight.target_roll = 0.0f;
    flight.target_pitch = 0.0f;
    flight.target_yaw = 0.0f;
    flight.throttle = 0.15f;

    // Основной цикл
    while(1) {

        if (MPU_Update()) {
            flight.roll = MPU_GetRoll();
            flight.pitch = MPU_GetPitch();
            flight.yaw = MPU_GetYaw();

            // PID вычисления
            float roll_out = PID_Compute(&pid_roll, flight.target_roll, flight.roll);
            float pitch_out = PID_Compute(&pid_pitch, flight.target_pitch, flight.pitch);
            float yaw_out = PID_Compute(&pid_yaw, flight.target_yaw, flight.yaw);

            // Миксер
            motor1 = flight.throttle - roll_out + pitch_out - yaw_out;
            motor2 = flight.throttle + roll_out + pitch_out + yaw_out;
            motor3 = flight.throttle - roll_out - pitch_out - yaw_out;
            motor4 = flight.throttle + roll_out - pitch_out + yaw_out;

            // Ограничение
            motor1 = (motor1 > 1.0f) ? 1.0f : (motor1 < 0.0f) ? 0.0f : motor1;
            motor2 = (motor2 > 1.0f) ? 1.0f : (motor2 < 0.0f) ? 0.0f : motor2;
            motor3 = (motor3 > 1.0f) ? 1.0f : (motor3 < 0.0f) ? 0.0f : motor3;
            motor4 = (motor4 > 1.0f) ? 1.0f : (motor4 < 0.0f) ? 0.0f : motor4;

            // Отправка на моторы
            PWM_SetDuty(1, motor1);
            PWM_SetDuty(2, motor2);
            PWM_SetDuty(3, motor3);
            PWM_SetDuty(4, motor4);
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

            tx_packet.start_byte = 0xAA;
            tx_packet.packet_id = 5;
            tx_packet.roll = roll_out;
            tx_packet.pitch = pitch_out;
            tx_packet.yaw = yaw_out;
            tx_packet.throttle = 3.3;
        }

        if(EXTI_5 == 1){					//Если пришло сообщение от NRF
        	read_FIFO(data);
        	if(data[0] == 1){
        		flight.throttle+=0.01;
        	}else if(data[0] == 2){
        		flight.throttle-=0.01;
        	}

        	 tx_packet.size = sizeof(TelemetryPacket);
        	 if (send_dataFlow_NRF24((uint8_t*)&tx_packet, sizeof(TelemetryPacket))) {
        	        // Отправлено успешно
        	    }
        }

/*
//        UART_SendString("********* RECEIVER **********");
        UART_SendNewLine();
        print_reg();

        UART_Receive_Byte_Blocking(&huart2);
    	//while(LL_GPIO_IsInputPinSet(My_IRQ_GPIO_Port, My_IRQ_Pin)) {}
        //


    	print_reg();// Чтение данных из буфера приемника и сброс флага успешного приема пакета
    	UART_SendNewLine();
    	print_array(data, 3);
    	//UART_SendHex(Ss);
*/
    	HAL_Delay(100);  // ~100 Гц
    }
}
