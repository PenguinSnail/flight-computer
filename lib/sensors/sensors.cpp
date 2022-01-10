#include "sensors.h"

Adafruit_BMP280 bmp;
float ground_altitude;

void setupSensors() {
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
}

/*!
 * @return The approximate altitude above ground level in feet.
 */
float readAltitude() {
  return bmp.readAltitude() * 3.281 - ground_altitude;
}