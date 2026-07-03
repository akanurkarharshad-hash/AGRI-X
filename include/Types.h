/******************************************************************************
 * AGRI-X Smart Agricultural Rover
 * File : Types.h
 ******************************************************************************/

#pragma once

#include <Arduino.h>

struct BatteryData
{
    float voltage;
    float percentage;
};

struct GPSData
{
    double latitude;
    double longitude;
    double altitude;
    bool valid;
};

struct EnvironmentData
{
    float temperature;
    float humidity;
};

struct NPKData
{
    uint16_t nitrogen;
    uint16_t phosphorus;
    uint16_t potassium;
};

enum class RoverState
{
    BOOTING,
    IDLE,
    MANUAL,
    AUTONOMOUS,
    SAMPLING,
    ERROR
};