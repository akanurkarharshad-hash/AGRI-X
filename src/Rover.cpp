#include "Rover.h"

constexpr uint8_t DRIVE_SPEED = 200;   // Forward / Backward
constexpr uint8_t TURN_SPEED  = 210;   // Left / Right

bool Rover::begin()
{
    return motors.begin();
}

void Rover::moveForward()
{
    motors.forward(currentSpeed);
}

void Rover::moveBackward()
{
    motors.reverse(currentSpeed);
}

void Rover::turnLeft()
{
    motors.left(currentSpeed);
}

void Rover::turnRight()
{
    motors.right(currentSpeed);
}

void Rover::rotateLeft()
{
    motors.left(TURN_SPEED);
}

void Rover::rotateRight()
{
    motors.right(TURN_SPEED);
}

void Rover::stop()
{
    motors.stop();
}

void Rover::setSpeed(uint8_t speed)
{
    currentSpeed = speed;
}

uint8_t Rover::getSpeed() const
{
    return currentSpeed;
}