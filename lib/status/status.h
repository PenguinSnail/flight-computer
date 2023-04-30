#ifndef status_h
#define status_h

#include <Arduino.h>

#include <config.h>

enum states
{
    CARD_ERROR = 0,
    FILE_ERROR,
    STANDBY,
    ARMED,
    ASCENT,
    DESCENT,
    GROUND,
};

extern states state;

void setupStatus();

void ledOn();
void ledOff();
void ledToggle();

void updateLed();

#endif