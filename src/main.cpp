#include <Arduino.h>

#include "Version.h"
#include "Constants.h"

void setup()
{
    Serial.begin(AGRIX::SERIAL_BAUD);

    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println(PROJECT_NAME);
    Serial.println("AI Smart Agricultural Rover");
    Serial.println("----------------------------------------");
    Serial.print("Version : ");
    Serial.println(PROJECT_VERSION);
    Serial.print("Author  : ");
    Serial.println(PROJECT_AUTHOR);
    Serial.print("Build   : ");
    Serial.print(BUILD_DATE);
    Serial.print(" ");
    Serial.println(BUILD_TIME);
    Serial.println("----------------------------------------");
    Serial.println("Firmware Boot Successful");
    Serial.println("========================================");
}

void loop()
{

}
