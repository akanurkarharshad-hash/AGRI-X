#include "MotorController.h"
#include "Pins.h"

constexpr uint8_t LEFT_CHANNEL  = 0;
constexpr uint8_t RIGHT_CHANNEL = 1;

constexpr uint32_t PWM_FREQ = 1000;
constexpr uint8_t PWM_RESOLUTION = 8;

bool MotorController::begin()
{
    pinMode(Pins::LEFT_IN1,OUTPUT);
    pinMode(Pins::LEFT_IN2,OUTPUT);

    pinMode(Pins::RIGHT_IN1,OUTPUT);
    pinMode(Pins::RIGHT_IN2,OUTPUT);

    ledcSetup(0, PWM_FREQ, PWM_RESOLUTION);
ledcAttachPin(Pins::LEFT_EN, 0);

ledcSetup(1, PWM_FREQ, PWM_RESOLUTION);
ledcAttachPin(Pins::RIGHT_EN, 1);

    stop();

    return true;
}

void MotorController::leftMotor(bool direction,uint8_t speed)
{
    digitalWrite(Pins::LEFT_IN1,direction);
    digitalWrite(Pins::LEFT_IN2,!direction);

    ledcWrite(0, speed);
}

void MotorController::rightMotor(bool direction,uint8_t speed)
{
    digitalWrite(Pins::RIGHT_IN1,direction);
    digitalWrite(Pins::RIGHT_IN2,!direction);

    ledcWrite(1, speed);
}

void MotorController::forward(uint8_t speed)
{
    leftMotor(true,speed);
    rightMotor(true,speed);
}

void MotorController::reverse(uint8_t speed)
{
    leftMotor(false,speed);
    rightMotor(false,speed);
}

void MotorController::left(uint8_t speed)
{
    leftMotor(false,speed);
    rightMotor(true,speed);
}

void MotorController::right(uint8_t speed)
{
    leftMotor(true,speed);
    rightMotor(false,speed);
}

void MotorController::stop()
{
    ledcWrite(0, 0);
ledcWrite(1, 0);

    digitalWrite(Pins::LEFT_IN1,LOW);
    digitalWrite(Pins::LEFT_IN2,LOW);

    digitalWrite(Pins::RIGHT_IN1,LOW);
    digitalWrite(Pins::RIGHT_IN2,LOW);
}