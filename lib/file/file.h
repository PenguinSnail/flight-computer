#ifndef file_h
#define file_h

#include <Arduino.h>
#include <SD.h>
#include <status.h>

#include <config.h>
#include <point.h>

#define SD_CS_PIN 3

bool startFile();
void writeDataPoint(DataPoint point);
void endFile();

#endif