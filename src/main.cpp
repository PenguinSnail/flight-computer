#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <SD.h>

#define SD_CS_PIN 3
#define BUTTON_PIN 0

#define STATUS_LED 1
#define CARD_LED 2

#define SAMPLERATE 50
#define STARTUP_DELAY 5000

Adafruit_BMP280 bmp;
float ground_altitude;

File data_file;
File stats_file;

bool status_led_state = true;
bool card_led_state = true;

bool recording = false;
unsigned long start_time = millis();
unsigned long last_interrupt_time = 0;

float last_altitudes[3] = { 0.0, 0.0, 0.0 };
unsigned long last_times[3] = { 0, 0, 0 };

float max_values[3] = { 0.0, 0.0, 0.0 };
unsigned long max_times[3] = { 0, 0, 0 };

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
  if (interrupt_time - last_interrupt_time > 200) {
    if (!recording) {
      File root = SD.open("/");
      int file_count = 0;

      while (true) {
        File entry = root.openNextFile();

        if (!entry) break;
        if (!entry.isDirectory()) {
          file_count++;
        }

        entry.close();
      }

      data_file = SD.open(String("flight") + file_count + "_data.csv", FILE_WRITE);
      stats_file = SD.open(String("flight") + file_count + "_stats.txt", FILE_WRITE);

      if (!data_file || !stats_file) {
        Serial.println("Failed to open file");

        while (true) {
          card_led_state = !card_led_state;
          digitalWrite(CARD_LED, card_led_state ? HIGH : LOW);
          delay(1000/2/2);
        };
      };

      start_time = millis();

      max_values[0] = readAltitudeFeet();
      max_times[0] = 0;
      max_values[1] = 0.0;
      max_times[1] = 0;
      max_values[2] = 0.0;
      max_times[2] = 0;

      data_file.println("time milliseconds, altitude feet");
    } else {
      data_file.close();

      stats_file.printf("Max altitude: %.2f ft at T+%.3f seconds", max_values[0], max_times[0] / 100.0);
      stats_file.printf("Max vertical velocity: %.2f ft/s (%.2f mph) at T+ %.3f seconds", max_values[1] * 1000, max_values[1] * 0.681818, max_times[1] / 100.0);
      stats_file.printf("Max vertical acceleration: %.2f ft/s^2 (%.2f Gs) at T+ %.3f seconds", max_values[2] * 1000 * 1000, max_values[2] * 0.03108095, max_times[2] / 100.0);
      stats_file.close();

      status_led_state = false;
      card_led_state= false;
    }

    recording = !recording;
  }

  last_interrupt_time = interrupt_time;
}

void setup() {
  // start serial for debugging
  Serial.begin(9600);
  // setup pin modes
  pinMode(STATUS_LED, OUTPUT);
  pinMode(CARD_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // turn on the status LEDs
  digitalWrite(STATUS_LED, HIGH);
  digitalWrite(CARD_LED, HIGH);

  delay(STARTUP_DELAY);

  // initialize the pressure sensor
  if (!bmp.begin(0x76)) {
    Serial.println("Failed to initialize BMP");

    // blink the led
    while (true) {
      status_led_state = !status_led_state;
      digitalWrite(STATUS_LED, status_led_state ? HIGH : LOW);
      delay(1000/3/2);
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

  status_led_state = false;
  digitalWrite(STATUS_LED, status_led_state ? HIGH : LOW);

  Serial.println("Altitude zeroed");

  // initialize the SD card
  if (SD.begin(SD_CS_PIN)) {
    Serial.println("Failed to initialize SD card");

    // blink the led
    while (true) {
      card_led_state = !card_led_state;
      digitalWrite(CARD_LED, card_led_state ? HIGH : LOW);
      delay(1000/2/3);
    };
  };
  Serial.println("SD card initialized");
  
  card_led_state = false;
  digitalWrite(CARD_LED, card_led_state ? HIGH : LOW);

  attachInterrupt(0, buttonHandler, LOW);
}

void loop() {
  if (recording) {
    if ((millis() / 500) % 2 == 0) {
      status_led_state = true;
      card_led_state = false;
    } else {
      status_led_state = false;
      card_led_state = true;
    }

    if (millis() % (1000/SAMPLERATE) == 0) {
      float altitude = readAltitudeFeet();
      unsigned long time = millis() - start_time;
      addToLastAltitudes(altitude, time);
      updateMaxStats();

      data_file.printf("%d, %.2f", time, altitude);
    }
  }

  digitalWrite(STATUS_LED, status_led_state ? HIGH : LOW);
  digitalWrite(CARD_LED, card_led_state ? HIGH : LOW);
}
