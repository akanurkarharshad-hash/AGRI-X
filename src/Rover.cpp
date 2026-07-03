#include "Rover.h"

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
    motors.left(currentSpeed);
}

void Rover::rotateRight()
{
    motors.right(currentSpeed);
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