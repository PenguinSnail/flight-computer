#include "file.h"

File data_file;

bool startFile()
{
    // initialize the SD card
    pinMode(SD_CS_PIN, OUTPUT);
    if (!SD.begin(SD_CS_PIN))
    {
        debugLog("Failed to initialize SD card");
        state = states::CARD_ERROR;
        return false;
    };
    debugLog("SD card initialized");

    // count the number of csv files (flights)
    File root = SD.open("/");
    int flight_count = 0;

    while (true)
    {
        File entry = root.openNextFile();
        if (!entry)
            break;
        if (!entry.isDirectory())
        {
            flight_count++;
        }

        entry.close();
    }

    // filenames
    char data_file_name[20];

    sprintf(data_file_name, "%03d-data.CSV", flight_count);

    // open files for writing
    debugLog("begin recording flight");
    data_file = SD.open(data_file_name, FILE_WRITE);

    // fail if files couldn't be opened
    if (!data_file)
    {
        debugLog("Failed to open file");
        state = states::FILE_ERROR;
        return false;
    };

    // csv column titles
    data_file.println("time,altitude,acceleration_x,acceleration_y,acceleration_z,rotation_x,rotation_y,rotation_z");
    return true;
}

void writeData(unsigned long time, float altitude, float ax, float ay, float az, float gx, float gy, float gz)
{
    data_file.printf("%.3f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", time / 1000.0, altitude, ax, ay, az, gx, gy, gz);
}
void writeDataPoint(DataPoint point)
{
    writeData(point.time, point.altitude, point.acceleration_x, point.acceleration_y, point.acceleration_z, point.rotation_x, point.rotation_y, point.rotation_z);
}

void endFile()
{
    data_file.close();
    SD.end();
}