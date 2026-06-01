#include "mpu_wrapper.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"

// Статические объекты (видны только в этом файле)
static MPU6050 mpu;
static uint8_t fifoBuffer[64];
static Quaternion q;
static VectorFloat gravity;
static float ypr[3];  // [yaw, pitch, roll]
static bool dmpReady = false;
static bool connected = false;

// Экспортируемые C-функции
extern "C" {

void MPU_Init(void) {
    mpu.initialize();
    connected = mpu.testConnection();

    if (connected) {
        // Инициализация DMP
        if (mpu.dmpInitialize() == 0) {
            mpu.setDMPEnabled(true);
            dmpReady = true;
        }
    }
}

bool MPU_TestConnection(void) {
    return connected;
}

bool MPU_Update(void) {
    if (!dmpReady) return false;

    if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        return true;
    }
    return false;
}

float MPU_GetRoll(void) {
    return ypr[2];   // Roll
}

float MPU_GetPitch(void) {
    return ypr[1];   // Pitch
}

float MPU_GetYaw(void) {
    return ypr[0];   // Yaw
}

}
