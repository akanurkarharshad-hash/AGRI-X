#include <Arduino.h>
#include "MotorController.h"

#include "Rover.h"

Rover rover;
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
  rover.begin();
    Serial.println();
Serial.println("===== ROVER LIBRARY TEST =====");

rover.setSpeed(80);

delay(2000);

Serial.println("Forward");
rover.moveForward();
delay(2000);

rover.stop();
delay(1000);

Serial.println("Backward");
rover.moveBackward();
delay(2000);

rover.stop();
delay(1000);

Serial.println("Left");
rover.turnLeft();
delay(1500);

rover.stop();
delay(1000);

Serial.println("Right");
rover.turnRight();
delay(1500);

rover.stop();

Serial.println("Rover Library OK");
}
void loop()
{
    // Future rover code will run here
}