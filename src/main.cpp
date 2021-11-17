#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <SD.h>

#define SD_CS_PIN 3
#define BUTTON_PIN 0

#define POWER_LED 2
#define STATUS_LED 1

#define SAMPLERATE 50
#define STARTUP_DELAY 0

enum states {
  STANDBY = 0,
  RECORDING,
  CARD_ERROR,
  FILE_ERROR
};

Adafruit_BMP280 bmp;
float ground_altitude;

File flight_directory;
File data_file;
File stats_file;

states state;

unsigned long start_time = millis();
unsigned long last_button_time = 0;

float last_altitudes[3] = { 0.0, 0.0, 0.0 };
unsigned long last_times[3] = { 0, 0, 0 };

float max_values[3] = { 0.0, 0.0, 0.0 };
unsigned long max_times[3] = { 0, 0, 0 };

bool status_led_state = false;

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

float readAltitudeFeet() {
  return bmp.readAltitude() * 3.281 - ground_altitude;
}

void addToLastAltitudes(float altitude, unsigned long time) {
  last_altitudes[0] = last_altitudes[1];
  last_altitudes[1] = last_altitudes[2];
  last_altitudes[2] = altitude;

  last_times[0] = last_times[1];
  last_times[1] = last_times[2];
  last_times[2] = time;
}

void updateMaxStats() {
  if (last_altitudes[2] > max_values[0]) {
    max_values[0] = last_altitudes[2];
    max_times[0] = last_times[2];
  }

  float velocity_distance = last_altitudes[2] - last_altitudes[1];
  unsigned long velocity_duration = last_times[2] - last_times[1];
  float velocity = velocity_distance / velocity_duration;

  if (velocity > max_values[1]) {
    max_values[1] = velocity;
    max_times[1] = last_times[2];
  }

  float previous_velocity_distance = last_altitudes[1] - last_altitudes[0];
  unsigned long previous_velocity_duration = last_times[1] - last_times[0];
  float previous_velocity = previous_velocity_distance / previous_velocity_duration;
  
  float acceleration_change_in_velocity = velocity - previous_velocity;
  unsigned long acceleration_duration = last_times[2] - last_times[1];
  float acceleration = acceleration_change_in_velocity / acceleration_duration;

  if (acceleration > max_values[2]) {
    max_values[2] = acceleration;
    max_times[2] = last_times[2];
  }
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
        if (entry.isDirectory()) {
          flight_count++;
        }

        entry.close();
      }

      // filenames
      char flight_directory_name[4];
      char data_file_name[17];
      char stats_file_name[17];

      sprintf(flight_directory_name, "%03d", flight_count);
      SD.mkdir(flight_directory_name);

      sprintf(data_file_name, "%s/%03d-data.CSV", flight_directory_name, flight_count);
      sprintf(stats_file_name, "%s/%03d-stat.TXT", flight_directory_name, flight_count);

      // open files for writing
      Serial.printf("begin recording flight %d\n", flight_count);
      data_file = SD.open(data_file_name, FILE_WRITE);
      stats_file = SD.open(stats_file_name, FILE_WRITE);

      // fail if files couldn't be opened
      if (!data_file || !stats_file) {
        Serial.println("Failed to open file");
        state = states::FILE_ERROR;
        return;
      };

      // set recording start time
      start_time = millis();

      // clear out maxes
      max_values[0] = readAltitudeFeet();
      max_times[0] = 0;
      max_values[1] = 0.0;
      max_times[1] = 0;
      max_values[2] = 0.0;
      max_times[2] = 0;

      // csv column titles
      data_file.println("time,altitude");

      state = states::RECORDING;
    } else {
      ledOn();
      // print out statistics
      stats_file.printf("Max altitude: %.2f ft at T+%.3f seconds\n", max_values[0], max_times[0] / 1000.0);
      stats_file.printf("Max vertical velocity: %.2f ft/s (%.2f mph) at T+ %.3f seconds\n", max_values[1] * 1000, max_values[1] * 1000 * 0.681818, max_times[1] / 1000.0);
      stats_file.printf("Max vertical acceleration: %.2f ft/s^2 (%.2f Gs) at T+ %.3f seconds\n", max_values[2] * 1000 * 1000, max_values[2] * 1000 * 1000 * 0.03108095, max_times[2] / 1000.0);
      stats_file.close();

      data_file.close();

      SD.end();
      ledOff();
      Serial.println("end recording");

      state = states::STANDBY;
    }
  }

  last_button_time = interrupt_time;
}

void setup() {
  // start serial for debugging
  Serial.begin(9600);
  // setup pin modes
  pinMode(SD_CS_PIN, OUTPUT);
  pinMode(POWER_LED, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
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

  state = states::STANDBY;
}

void loop() {
  // if the button is pressed
  if (digitalRead(0) == LOW) {
    buttonHandler();
  } else if (state == states::RECORDING) {
    // led blinking
    if ((millis() / 500) % 2 == 0) {
      ledOn();
    } else {
      ledOff();
    }

    // if a sample should be taken
    if ((millis() - start_time) % (1000/SAMPLERATE) == 0) {
      // read the altitude
      float altitude = readAltitudeFeet();
      // read time since recording started
      unsigned long time = millis() - start_time;
      // push the sample and update calculated statistics
      addToLastAltitudes(altitude, time);
      updateMaxStats();

      // log the data to the file
      data_file.printf("%.3f,%.2f\n", time / 1000.0, altitude);
    }
  } else if (state == states::CARD_ERROR) {
    // led blinking
    int ms_of_s = millis() % 1000;
    if ((ms_of_s > 0 && ms_of_s < 150) || (ms_of_s > 300 && ms_of_s < 450)) {
      ledOn();
    } else {
      ledOff();
    }
  } else if (state == states::FILE_ERROR) {
    // led blinking
    int ms_of_s = millis() % 1000;
    if ((ms_of_s > 0 && ms_of_s < 125) || (ms_of_s > 250 && ms_of_s < 375) || (ms_of_s > 500 && ms_of_s < 625)) {
      ledOn();
    } else {
      ledOff();
    }
  }
}
