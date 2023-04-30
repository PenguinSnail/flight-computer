#ifndef sensors_h
#define sensors_h

#include <Arduino.h>
#include <status.h>
#include <log.h>

#include <config.h>
#include <point.h>

#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>

void setupSensors();
float readAltitude();

DataPoint readDataPoint();

#endif