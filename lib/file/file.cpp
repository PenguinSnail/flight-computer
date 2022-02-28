#include "file.h"

File data_file;

bool startFile() {
  // initialize the SD card
  pinMode(SD_CS_PIN, OUTPUT);
  if (!SD.begin(SD_CS_PIN)) {
    #ifdef DEBUG
      Serial.println("Failed to initialize SD card");
    #endif
    state = states::CARD_ERROR;
    return false;
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
  char data_file_name[20];

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
    state = states::FILE_ERROR;
    return false;
  };

  // csv column titles
  data_file.println("time,altitude");
  return true;
}

void writeData(unsigned long time, float altitude) {
  data_file.printf("%.3f,%.2f\n", time / 1000.0, altitude);
}
void writeDataPoint(DataPoint point) {
  writeData(point.time, point.altitude);
}

void endFile() {
  data_file.close();
  SD.end();
}