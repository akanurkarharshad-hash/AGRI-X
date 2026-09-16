#pragma once

#include <Arduino.h>
#include <TinyGPS++.h>

class GPSManager
{
public:

    bool begin();

    void update();

    double getLatitude() const;

    double getLongitude() const;

    double getAltitude() const;

    double getSpeed() const;

    uint32_t getSatellites() const;

    bool hasFix() const;

private:

    TinyGPSPlus gps;

    HardwareSerial gpsSerial = HardwareSerial(2);

    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    double speed = 0.0;

    uint32_t satellites = 0;

    bool fix = false;
};