#pragma once

#include <Arduino.h>

namespace AGRIX
{
    constexpr uint32_t SERIAL_BAUD = 115200;

    constexpr uint16_t JSON_BUFFER_SIZE = 2048;

    constexpr uint8_t DEFAULT_SPEED = 180;

    constexpr uint8_t MAX_SPEED = 255;

    constexpr float LOW_BATTERY = 10.8f;

    constexpr float CRITICAL_BATTERY = 10.2f;

    constexpr char PROJECT[] = "AGRI-X";
}
