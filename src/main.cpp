#include <Arduino.h>
#include "MotorController.h"

MotorController motors;
#include "Version.h"
#include "Constants.h"
#include "Pins.h"
#include "Config.h"
#include "Types.h"

void initializeGPIO()
{
    pinMode(Pins::STATUS_LED, OUTPUT);

    digitalWrite(Pins::STATUS_LED, LOW);
}

void setup()
{
    motors.begin();
    Serial.println();
Serial.println("========== MOTOR TEST ==========");

delay(2000);

Serial.println("Forward");
motors.forward(80);
delay(2000);

motors.stop();
Serial.println("Stop");
delay(1000);

Serial.println("Reverse");
motors.reverse(80);
delay(2000);

motors.stop();
Serial.println("Stop");
delay(1000);

Serial.println("Left");
motors.left(80);
delay(1500);

motors.stop();
delay(1000);

Serial.println("Right");
motors.right(80);
delay(1500);

motors.stop();

Serial.println("Motor Test Complete");
    Serial.begin(AGRIX::SERIAL_BAUD);

    delay(1000);

    initializeGPIO();

    Serial.println();
    Serial.println("========================================");
    Serial.println(PROJECT_NAME);
    Serial.println("AI Smart Agricultural Rover");
    Serial.println("----------------------------------------");
    Serial.print("Version : ");
    Serial.println(PROJECT_VERSION);
    Serial.print("Author  : ");
    Serial.println(PROJECT_AUTHOR);
    Serial.println("----------------------------------------");

    Serial.println("GPIO Initialized");
    Serial.println("System Ready");

    digitalWrite(Pins::STATUS_LED, HIGH);
}

void loop()
{
}