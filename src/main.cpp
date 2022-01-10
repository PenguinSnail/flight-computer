#include <Arduino.h>

#include <file.h>
#include <status.h>
#include <sensors.h>

//#define DEBUG

#define BUTTON_PIN 0
#define POWER_LED 2

#define SAMPLERATE 50
#define STARTUP_DELAY 0

unsigned long start_time = millis();
unsigned long last_button_time = 0;

void setup() {
  setupStatus();
  #ifdef DEBUG
    // start serial for debugging
    Serial.begin(9600);
  #endif
  // setup pin modes
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // power led on
  pinMode(POWER_LED, OUTPUT);
  digitalWrite(POWER_LED, HIGH);

  // delay startup to allow serial connections,
  // then setup the sensors
  ledOn();
  delay(STARTUP_DELAY);
  setupSensors();
  ledOff();
}

void loop() {
  // if the button is pressed
  if (digitalRead(0) == LOW) {
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if (interrupt_time - last_button_time > 200) {
      // if we're not already recording
      if (state != states::RECORDING) {
        // attempt to start the file
        state = startFile();
        // set recording start time
        start_time = millis();
      } else {
        // else if we are recording,
        // trigger the led and end the file
        ledOn();
        endFile();
        ledOff();
        // return to standby
        state = states::STANDBY;

        #ifdef DEBUG
          Serial.println("end recording");
        #endif
      }
    }

    last_button_time = interrupt_time;
  } else if (state == states::RECORDING) {
    // if a sample should be taken
    if ((millis() - start_time) % (1000/SAMPLERATE) == 0) {
      // read time since recording started
      unsigned long time = millis() - start_time;
      // read the altitude
      float altitude = readAltitude();

      // log the data to the file
      writeData(time, altitude);
    }
  }

  updateLed();
}
