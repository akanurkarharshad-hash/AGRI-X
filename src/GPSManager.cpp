#include "GPSManager.h"
#include "TimingConfig.h"

bool GPSManager::begin()
{
    gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

    Serial.println("GPS Initialized");

    return true;
}

void GPSManager::update()
{
    uint8_t bytesRead = 0;
    while (gpsSerial.available() && bytesRead < TimingConfig::GPS_BYTES_PER_LOOP)
    {
        gps.encode(gpsSerial.read());
        ++bytesRead;
    }

    if (gps.location.isUpdated())
    {
        latitude = gps.location.lat();
        longitude = gps.location.lng();

        altitude = gps.altitude.meters();

        speed = gps.speed.kmph();

        satellites = gps.satellites.value();

        fix = gps.location.isValid();
    }
}

double GPSManager::getLatitude() const
{
    return latitude;
}

double GPSManager::getLongitude() const
{
    return longitude;
}

double GPSManager::getAltitude() const
{
    return altitude;
}

double GPSManager::getSpeed() const
{
    return speed;
}

uint32_t GPSManager::getSatellites() const
{
    return satellites;
}

bool GPSManager::hasFix() const
{
    return fix;
}
