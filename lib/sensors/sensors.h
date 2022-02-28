#ifndef sensors_h
#define sensors_h

#include <Arduino.h>
#include <status.h>

#include <config.h>
#include <point.h>

#ifdef SENSOR_BARO
  #include <Adafruit_BMP280.h>
#endif

void setupSensors();
#ifdef SENSOR_BARO
  float readAltitude();
#endif

DataPoint readDataPoint();

#endif