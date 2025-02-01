# Adafruit Sensor Calibration

Added support for external flash memory for Xiao NRF52840 Sense (P25Q16H) by adding the following section to `lib/Adafruit Sensor Calibration/Adafruit_Sensor_Calibration_SDFat.cpp`:

```cpp
#ifdef ADAFRUIT_SENSOR_CALIBRATION_USE_FLASH
  #ifndef ARDUINO_Seeed_XIAO_nRF52840_Sense
    if (!flash.begin())
    {
      return false;
    }
    Serial.print("JEDEC ID: ");
    Serial.println(flash.getJEDECID(), HEX);
    Serial.print("Flash size: ");
    Serial.println(flash.size());

    if (!fatfs.begin(&flash))
    {
      Serial.println("Error, failed to mount newly formatted filesystem!");
      Serial.println("Was it formatted with the fatfs_format example?");
      return false;
    }
    theFS = &fatfs;
  #endif // XIAO_SENSOR_CALIBRATION_USE_FLASH
#ifdef ARDUINO_Seeed_XIAO_nRF52840_Sense
    // Built from the P25Q16H datasheet.
    SPIFlash_Device_t const P25Q16H_XIAO{
        .total_size = (1UL << 21), // 2MiB
        .start_up_time_us = 10000, // Don't know where to find that value

        .manufacturer_id = 0x85,
        .memory_type = 0x60,
        .capacity = 0x15,

        .max_clock_speed_mhz = 55,
        .quad_enable_bit_mask = 0x02,     // Datasheet p. 27
        .has_sector_protection = 1,       // Datasheet p. 27
        .supports_fast_read = 1,          // Datasheet p. 29
        .supports_qspi = 1,               // Obviously
        .supports_qspi_writes = 1,        // Datasheet p. 41
        .write_status_register_split = 1, // Datasheet p. 28
        .single_status_byte = 0,          // 2 bytes
        .is_fram = 0,                     // Flash Memory
    };
    if (!flash.begin(&P25Q16H_XIAO, 1))
    {
      return false;
    }
    Serial.print("JEDEC ID: ");
    Serial.println(flash.getJEDECID(), HEX);
    Serial.print("Flash size: ");
    Serial.println(flash.size());

    if (!fatfs.begin(&flash))
    {
      Serial.println("Error, failed to mount newly formatted filesystem!");
      Serial.println("Was it formatted with the fatfs_format example?");
      return false;
    }
    theFS = &fatfs;
  #endif // XIAO_SENSOR_CALIBRATION_USE_FLASH
#else
    return false;
#endif
```

## Explanation of `main.cpp`

The `main.cpp` file initializes and configures the sensors and the external flash memory for the Xiao NRF52840 Sense. It sets up the Adafruit Sensor Lab and the Adafruit AHRS (Attitude and Heading Reference System) for sensor fusion. The code also includes support for the ICM20948 sensor, which is now supported in the Adafruit Sensor Lab.

### Key Features:
- Initializes the external flash memory for storing calibration data.
- Configures the ICM20948 sensor for accelerometer, gyroscope, and magnetometer readings.
- Sets up the Adafruit Sensor Lab for easy sensor management.
- Implements sensor fusion using the Adafruit AHRS library.

### ICM20948 Support
The ICM20948 sensor is now supported in the Adafruit Sensor Lab, allowing for easy integration and management of this 9-DoF (Degrees of Freedom) sensor.