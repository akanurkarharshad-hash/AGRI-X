#include "WiFiConfig.h"
#include <Arduino.h>
#include <Preferences.h>

Preferences preferences;

bool WiFiConfig::begin()
{
    return preferences.begin("wifi", false);
}

bool WiFiConfig::hasCredentials()
{
    return preferences.isKey("ssid");
}

String WiFiConfig::getSSID()
{
    return preferences.getString("ssid", "");
}

String WiFiConfig::getPassword()
{
    return preferences.getString("password", "");
}

void WiFiConfig::saveCredentials(const String& ssid, const String& password)
{
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
}

void WiFiConfig::clearCredentials()
{
    preferences.clear();
}