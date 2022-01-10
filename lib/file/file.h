#ifndef file_h
#define file_h

#include <Arduino.h>
#include <SD.h>
#include <status.h>

//#define DEBUG

#define SD_CS_PIN 3

states startFile();
void writeData(unsigned long time, float alt);
void endFile();

#endif