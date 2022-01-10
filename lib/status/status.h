#ifndef status_h
#define status_h

#include <Arduino.h>

//#define DEBUG

#define STATUS_LED 1

enum states {
  STANDBY = 0,
  RECORDING,
  CARD_ERROR,
  FILE_ERROR
};

extern states state;

void setupStatus();

void ledOn();
void ledOff();
void ledToggle();

void updateLed();

#endif