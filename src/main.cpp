#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <SD.h>

#include <status.h>

#define SD_CS_PIN 3
#define BUTTON_PIN 0

#define POWER_LED 2

#define SAMPLERATE 50
#define STARTUP_DELAY 0

Adafruit_BMP280 bmp;
float ground_altitude;

File data_file;

unsigned long start_time = millis();
unsigned long last_button_time = 0;

float readAltitudeFeet() {
  return bmp.readAltitude() * 3.281 - ground_altitude;
}

void setup() {
  setupStatus();
  // start serial for debugging
  Serial.begin(9600);
  // setup pin modes
  pinMode(SD_CS_PIN, OUTPUT);
  pinMode(POWER_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // turn on the power and status LEDs
  digitalWrite(POWER_LED, HIGH);
  ledOn();

  // delay startup to allow serial connections
  delay(STARTUP_DELAY);

  // initialize the pressure sensor
  if (!bmp.begin(0x76)) {
    Serial.println("Failed to initialize BMP");

    // lock execution and blink the led
    while (true) {
      if (millis() % 333 == 0) {
        ledToggle();
      }
    };
  };
  Serial.println("BMP initialized");

  // zero the altimeter reading
  float altitude_reset = 0;
  for (int i = 0; i < 10; i++) {
    delay(100);
    altitude_reset += readAltitudeFeet();
  }
  ground_altitude = altitude_reset / 10;
  Serial.println("Altitude zeroed");

  // turn off the status led once sensor setup is complete
  ledOff();
}

void buttonHandler() {
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_button_time > 200) {
    if (state != states::RECORDING) {
      // initialize the SD card
      if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Failed to initialize SD card");
        state = states::CARD_ERROR;
        return;
      };
      Serial.println("SD card initialized");

      // count the number of csv files (flights)
      File root = SD.open("/");
      int flight_count = 0;

      while (true) {
        File entry = root.openNextFile();
        if (!entry) break;
        if (!entry.isDirectory()) {
          flight_count++;
        }

        entry.close();
      }

      // filenames
      char data_file_name[17];

      sprintf(data_file_name, "%03d-data.CSV", flight_count);

      // open files for writing
      Serial.printf("begin recording flight %d\n", flight_count);
      data_file = SD.open(data_file_name, FILE_WRITE);

      // fail if files couldn't be opened
      if (!data_file) {
        Serial.println("Failed to open file");
        state = states::FILE_ERROR;
        return;
      };

      // set recording start time
      start_time = millis();

      // csv column titles
      data_file.println("time,altitude");

      state = states::RECORDING;
    } else {
      ledOn();

      data_file.close();
      SD.end();

      ledOff();
      Serial.println("end recording");

      state = states::STANDBY;
    }
  }

  last_button_time = interrupt_time;
}

void loop() {
  // if the button is pressed
  if (digitalRead(0) == LOW) {
    buttonHandler();
  } else if (state == states::RECORDING) {
    // if a sample should be taken
    if ((millis() - start_time) % (1000/SAMPLERATE) == 0) {
      // read the altitude
      float altitude = readAltitudeFeet();
      // read time since recording started
      unsigned long time = millis() - start_time;

      // log the data to the file
      data_file.printf("%.3f,%.2f\n", time / 1000.0, altitude);
    }
  }

  updateLed();
}
