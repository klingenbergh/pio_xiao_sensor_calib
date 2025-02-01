#include <Arduino.h>
#include <bluefruit.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_SensorLab.h>
#include <Adafruit_Sensor_Calibration.h>
#include <Adafruit_AHRS.h>

// -----------------------------------------------------------------------------
// 1) Pick your filter -- we use Adafruit_Madgwick here
//    Slower filter => more stable but more CPU usage. 
//    Madgwick is a good mid-point.
// -----------------------------------------------------------------------------
//Adafruit_Madgwick filter;  // good compromise of speed & performance
Adafruit_Mahony filter;  // or Adafruit_NXPSensorFusion filter;

// Tweak the filter rate
#define FILTER_UPDATE_RATE_HZ 100
static uint32_t lastUpdate = 0;

// -----------------------------------------------------------------------------
// 2) SensorLab + Calibration
// -----------------------------------------------------------------------------
Adafruit_SensorLab lab;
Adafruit_Sensor *accelerometer = nullptr, *gyroscope = nullptr, *magnetometer = nullptr;

#if defined(ADAFRUIT_SENSOR_CALIBRATION_USE_EEPROM)
  Adafruit_Sensor_Calibration_EEPROM cal;
#else
  Adafruit_Sensor_Calibration_SDFat cal;
#endif

// -----------------------------------------------------------------------------
// 3) GYRO OFFSET STORAGE
// We’ll measure the gyro offset at startup when the device is still.
// Then we'll subtract that offset from each gyro reading so at rest => ~0 dps
// -----------------------------------------------------------------------------
static float gyroOffsetX = 0.0f, gyroOffsetY = 0.0f, gyroOffsetZ = 0.0f;
static bool  gyroCalibrated = false;

void calibrateGyro(int numSamples);
// -----------------------------------------------------------------------------
// 4) Setup
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial) yield();

  Serial.println(F("Sensor Lab - IMU AHRS (Madgwick)"));

  // Initialize SensorLab
  lab.begin();

  // Load calibration (mag/accel offsets, etc.)
  if (!cal.begin()) {
    Serial.println("Failed to init calibration helper");
  } else if (!cal.loadCalibration()) {
    Serial.println("No calibration loaded/found");
  }
  cal.printSavedCalibration();
  // 4a) Acquire sensors
  magnetometer = lab.getMagnetometer();
  if (!magnetometer) {
    Serial.println(F("No magnetometer found!"));
    while (1) yield();
  }
  gyroscope = lab.getGyroscope();
  if (!gyroscope) {
    Serial.println(F("No gyroscope found!"));
    while (1) yield();
  }
  accelerometer = lab.getAccelerometer();
  if (!accelerometer) {
    Serial.println(F("No accelerometer found!"));
    while (1) yield();
  }

  // 4b) Print sensor details (optional)
  // accelerometer->printSensorDetails();
  // gyroscope->printSensorDetails();
  // magnetometer->printSensorDetails();

  // 4c) Initialize filter
  filter.begin(FILTER_UPDATE_RATE_HZ);
  // Increase the Madgwick "beta" to more aggressively correct drift. 
  // Default is ~0.1...0.3. Try 0.6 for strong magnet correction:
  // filter.setBeta(0.2f);  

  // 4d) Faster I2C if supported
  Wire.setClock(400000);

  // 4e) Let’s do a quick GYRO calibration
  Serial.println("Keep the device still for 2 seconds to calibrate the gyro...");
  delay(2000);
  calibrateGyro(200); // read ~200 times, ~2s
  Serial.println("Gyro offset done. Starting loop!");
}

// -----------------------------------------------------------------------------
// 5) Main Loop
// -----------------------------------------------------------------------------
void loop() {
  // We want the filter to update at ~FILTER_UPDATE_RATE_HZ = 100 Hz 
  if ((millis() - lastUpdate) < (1000 / FILTER_UPDATE_RATE_HZ)) {
    return;
  }
  lastUpdate = millis();

  // 5a) Acquire sensor events
  sensors_event_t accel, gyro, mag;
  accelerometer->getEvent(&accel);
  gyroscope->getEvent(&gyro);
  magnetometer->getEvent(&mag);

  // 5b) Calibrate them using Adafruit_Sensor_Calibration
  cal.calibrate(mag);
  cal.calibrate(accel);
  cal.calibrate(gyro);

  // 5c) Convert gyro rad/s => deg/s, then subtract offset
  float gx = gyro.gyro.x * SENSORS_RADS_TO_DPS - gyroOffsetX;
  float gy = gyro.gyro.y * SENSORS_RADS_TO_DPS - gyroOffsetY;
  float gz = gyro.gyro.z * SENSORS_RADS_TO_DPS - gyroOffsetZ;

  // 5d) Update the Madgwick filter
  filter.update(gx, gy, gz,
                accel.acceleration.x,
                accel.acceleration.y,
                accel.acceleration.z,
                mag.magnetic.x,
                mag.magnetic.y,
                mag.magnetic.z);

  // 5e) Print out heading [0..360) 
  float heading = filter.getYaw(); // Madgwick library returns [0..360) = yaw+180
  heading = fmodf(heading, 360.0f);
  if (heading < 0) heading += 360.0f; 

  Serial.print("Heading: "); Serial.println(heading, 1);
}

// -----------------------------------------------------------------------------
// 6) GYRO Calibration 
// -----------------------------------------------------------------------------
void calibrateGyro(int numSamples) {
  // Zero out
  gyroOffsetX = 0;
  gyroOffsetY = 0;
  gyroOffsetZ = 0;

  // gather
  for (int i=0; i<numSamples; i++) {
    sensors_event_t g;
    gyroscope->getEvent(&g);
    // convert to deg/s
    gyroOffsetX += (g.gyro.x * SENSORS_RADS_TO_DPS);
    gyroOffsetY += (g.gyro.y * SENSORS_RADS_TO_DPS);
    gyroOffsetZ += (g.gyro.z * SENSORS_RADS_TO_DPS);
    delay(10); 
  }
  gyroOffsetX /= numSamples;
  gyroOffsetY /= numSamples;
  gyroOffsetZ /= numSamples;

  Serial.print("Calculated gyro offsets (deg/s): ");
  Serial.print(gyroOffsetX, 2); Serial.print(", ");
  Serial.print(gyroOffsetY, 2); Serial.print(", ");
  Serial.print(gyroOffsetZ, 2); Serial.println();
}

