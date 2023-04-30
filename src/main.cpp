#include <Arduino.h>
#include <CircularBuffer.h>

#include <log.h>
#include <file.h>
#include <status.h>
#include <sensors.h>

#include "config.h"
#include "point.h"

unsigned long start_time = millis();
unsigned long last_button_time = 0;

CircularBuffer<DataPoint, ASCENT_SAMPLERATE * 2> buffer;

void setup()
{
    setupStatus();
    setupLog();
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

void loop()
{
    // if the button is pressed
    if (digitalRead(0) == LOW)
    {
        unsigned long interrupt_time = millis();
        // If interrupts come faster than 200ms, assume it's a bounce and ignore
        if (interrupt_time - last_button_time > 200)
        {
            // if we're not already recording
            if (state <= states::STANDBY)
            {
                // attempt to start the file
                if (startFile())
                {
#if defined LIFTOFF_ALTITUDE || defined LIFTOFF_ACCELERATION
                    state = states::ARMED;
#else
                    state = states::ASCENT;
#endif
                    // set recording start time
                    start_time = millis();
                }
            }
            else
            {
                // else if we are recording,
                // trigger the led and end the file
                ledOn();
                endFile();
                ledOff();
                // return to standby
                state = states::STANDBY;

                debugLog("end recording");
            }
        }

        last_button_time = interrupt_time;
    }
    else if (state == states::ARMED)
    {

        debugLog("Armed!");
        // if a sample should be taken
        if ((millis() - start_time) % (1000 / ASCENT_SAMPLERATE) == 0)
        {
            DataPoint point = readDataPoint();
            point.time = millis() - start_time;

            // log the data to the file
            buffer.push(point);
        }

        int launch_conditions_required = 0;
        int launch_conditions_met = 0;

#if defined LIFTOFF_ALTITUDE
        launch_conditions_required++;
        if (readAltitude() >= LIFTOFF_ALTITUDE)
            launch_conditions_met++;
#endif
#if defined LIFTOFF_ACCELERATION
        launch_conditions_required++;
        if (true)
            launch_conditions_met++;
#endif

        if (launch_conditions_met == launch_conditions_required)
        {

            debugLog("launch detected!");
            state = states::ASCENT;
            unsigned long offset = millis() - start_time;

            debugLog("dumping buffer to file...");

            while (!buffer.isEmpty())
            {
                DataPoint point = buffer.pop();
                point.time = point.time - offset;
                writeDataPoint(point);
            }

            start_time = millis();
        }
    }
    else if (state == states::ASCENT)
    {
        // if a sample should be taken
        if ((millis() - start_time) % (1000 / ASCENT_SAMPLERATE) == 0)
        {
            DataPoint point = readDataPoint();
            // read time since recording started
            point.time = millis() - start_time;

            // log the data to the file
            writeDataPoint(point);
        }
    }

    updateLed();
}
