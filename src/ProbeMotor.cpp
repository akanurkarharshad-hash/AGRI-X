#include "ProbeMotor.h"
#include "Pins.h"

constexpr uint8_t PROBE_PWM_CHANNEL = 2;
constexpr uint32_t PWM_FREQ = 5000;
constexpr uint8_t PWM_RES = 8;

bool ProbeMotor::begin()
{
    pinMode(Pins::PROBE_IN3, OUTPUT);
    pinMode(Pins::PROBE_IN4, OUTPUT);

    ledcSetup(PROBE_PWM_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(Pins::PROBE_EN, PROBE_PWM_CHANNEL);

    stop();

    return true;
}

void ProbeMotor::run(bool direction, uint8_t speed)
{
    digitalWrite(Pins::PROBE_IN3, direction);
    digitalWrite(Pins::PROBE_IN4, !direction);

    ledcWrite(PROBE_PWM_CHANNEL, speed);
}

void ProbeMotor::insert()
{
    run(false, 255);
}

void ProbeMotor::retract()
{
    run(true, 255);
}

void ProbeMotor::stop()
{
    ledcWrite(PROBE_PWM_CHANNEL, 0);

    digitalWrite(Pins::PROBE_IN3, LOW);
    digitalWrite(Pins::PROBE_IN4, LOW);
}