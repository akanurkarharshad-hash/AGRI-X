#pragma once

#include <Arduino.h>

class WiFiConfig
{
public:
    bool begin();

    bool hasCredentials();

    String getSSID();
    String getPassword();

    void saveCredentials(const String& ssid, const String& password);

    void clearCredentials();
};