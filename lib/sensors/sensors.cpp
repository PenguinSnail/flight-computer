#include "sensors.h"

#ifdef SENSOR_BARO
  Adafruit_BMP280 bmp;
  float ground_altitude;
#endif

void setupSensors() {
  #ifdef SENSOR_BARO
    // initialize the pressure sensor
    if (!bmp.begin(0x76)) {
      #ifdef DEBUG
        Serial.println("Failed to initialize BMP");
      #endif

      // lock execution and blink the led
      while (true) {
        if (millis() % 333 == 0) {
          ledToggle();
        }
      };
    };
    #ifdef DEBUG
      Serial.println("BMP initialized");
    #endif

    // zero the altimeter reading
    float altitude_reset = 0;
    for (int i = 0; i < 10; i++) {
      delay(100);
      altitude_reset += readAltitude();
    }
    ground_altitude = altitude_reset / 10;
    #ifdef DEBUG
      Serial.println("Altitude zeroed");
    #endif
  #endif
  #if defined SENSOR_ACCEL || defined SENSOR_GYRO
    
  #endif
}

#ifdef SENSOR_BARO
  /*!
   * @return The approximate altitude above ground level in feet.
   */
  float readAltitude() {
    return bmp.readAltitude() * 3.281 - ground_altitude;
  }
#endif

DataPoint readDataPoint() {
  DataPoint point;
  point.altitude = 0;
  #ifdef SENSOR_BARO
    point.altitude = readAltitude();
  #endif
  return point;
}