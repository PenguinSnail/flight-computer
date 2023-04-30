#include "sensors.h"

Adafruit_BMP280 bmp;
Adafruit_MPU6050 mpu;
float ground_altitude;
sensors_event_t accel, gyro, temp;

void setupSensors()
{
    // initialize the pressure sensor
    if (!bmp.begin(0x76))
    {
        debugLog("Failed to initialize BMP");

        // lock execution and blink the led
        while (true)
        {
            if (millis() % 333 == 0)
            {
                ledToggle();
            }
        };
    };
    debugLog("BMP initialized");
    // initialize the accelerometer/gyroscope
    if (!mpu.begin())
    {
        debugLog("Failed to initialize MPU");

        // lock execution and blink the led
        while (true)
        {
            if (millis() % 333 == 0)
            {
                ledToggle();
            }
        };
    };
    debugLog("MPU initialized");

    // zero the altimeter reading
    float altitude_reset = 0;
    for (int i = 0; i < 10; i++)
    {
        delay(100);
        altitude_reset += readAltitude();
    }
    ground_altitude = altitude_reset / 10;
    debugLog("Altitude zeroed");

    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
    debugLog("Sensor ranges set");
}

/*!
 * @return The approximate altitude above ground level in feet.
 */
float readAltitude()
{
    return bmp.readAltitude() * 3.281 - ground_altitude;
}

DataPoint readDataPoint()
{
    DataPoint point;
    point.altitude = readAltitude();
    mpu.getEvent(&accel, &gyro, &temp);
    point.acceleration_x = accel.acceleration.x;
    point.acceleration_y = accel.acceleration.y;
    point.acceleration_z = accel.acceleration.z;
    point.rotation_x = gyro.gyro.x;
    point.rotation_y = gyro.gyro.y;
    point.rotation_z = gyro.gyro.z;
    return point;
}