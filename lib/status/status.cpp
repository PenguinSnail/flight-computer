#include <Arduino.h>
#include "status.h"

#define STATUS_LED 1

states state = states::STANDBY;
bool status_led_state = false;

void setupStatus() {
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, status_led_state ? HIGH : LOW);
}

void ledOn() {
  status_led_state = true;
  digitalWrite(STATUS_LED, status_led_state ? HIGH : LOW);
}
void ledOff() {
  status_led_state = false;
  digitalWrite(STATUS_LED, status_led_state ? HIGH : LOW);
}
void ledToggle() {
  status_led_state = !status_led_state;
  digitalWrite(STATUS_LED, status_led_state ? HIGH : LOW);
}

void updateLed() {
  int ms_of_s = millis() % 1000;
  
  if (state == states::STANDBY) {
    ledOff();
  } else if (state == states::RECORDING) {
    if ((millis() / 500) % 2 == 0) {
      ledOn();
    } else {
      ledOff();
    }
  } else if (state == states::CARD_ERROR) {
    if ((ms_of_s > 0 && ms_of_s < 150) || (ms_of_s > 300 && ms_of_s < 450)) {
      ledOn();
    } else {
      ledOff();
    }
  } else if (state == states::FILE_ERROR) {
    if ((ms_of_s > 0 && ms_of_s < 125) || (ms_of_s > 250 && ms_of_s < 375) || (ms_of_s > 500 && ms_of_s < 625)) {
      ledOn();
    } else {
      ledOff();
    }
  }
}
