#include <Adafruit_ICM20948.h>
Adafruit_ICM20948 icm209;
// For (newer) Feather Sense with LSM6DS3TR-C, use this:
//#include <Adafruit_LSM6DS3TRC.h>
// Adafruit_LSM6DS3TRC lsm6ds;

bool init_sensors(void) {
  if (!icm209.begin_I2C()) {
    return false;
  }
  accelerometer = icm209.getAccelerometerSensor();
  gyroscope = icm209.getGyroSensor();
  magnetometer = &icm209;

  return true;
}

void setup_sensors(void) {
  // set lowest range
  icm209.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  icm209.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
  icm209.setRange(LIS3MDL_RANGE_4_GAUSS);

  // set slightly above refresh rate
  icm209.setAccelDataRate(LSM6DS_RATE_104_HZ);
  icm209.setGyroDataRate(LSM6DS_RATE_104_HZ);
  icm209.setDataRate(LIS3MDL_DATARATE_1000_HZ);
  icm209.setPerformanceMode(LIS3MDL_MEDIUMMODE);
  icm209.setOperationMode(LIS3MDL_CONTINUOUSMODE);
}
