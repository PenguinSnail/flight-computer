#ifndef sensors_h
#define sensors_h

#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <status.h>

//#define DEBUG

void setupSensors();
float readAltitude();

#endif