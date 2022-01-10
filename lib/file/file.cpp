#include "file.h"

File data_file;

states startFile() {
  // initialize the SD card
  pinMode(SD_CS_PIN, OUTPUT);
  if (!SD.begin(SD_CS_PIN)) {
    #ifdef DEBUG
      Serial.println("Failed to initialize SD card");
    #endif
    return states::CARD_ERROR;
  };
  #ifdef DEBUG
    Serial.println("SD card initialized");
  #endif

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
  #ifdef DEBUG
    Serial.printf("begin recording flight %d\n", flight_count);
  #endif
  data_file = SD.open(data_file_name, FILE_WRITE);

  // fail if files couldn't be opened
  if (!data_file) {
    #ifdef DEBUG
      Serial.println("Failed to open file");
    #endif
    return states::FILE_ERROR;
  };

  // csv column titles
  data_file.println("time,altitude");

  return states::RECORDING;
}

void writeData(unsigned long time, float altitude) {
  data_file.printf("%.3f,%.2f\n", time / 1000.0, altitude);
}

void endFile() {
  data_file.close();
  SD.end();
}