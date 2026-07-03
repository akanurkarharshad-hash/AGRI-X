/******************************************************************************
 * AGRI-X Smart Agricultural Rover
 * File : Pins.h
 * Description : Permanent GPIO Mapping
 ******************************************************************************/

#pragma once

#include <Arduino.h>

namespace Pins
{
   // ==========================
// Drive Motor Driver (L298N #1)
// ==========================

// Left Side
constexpr uint8_t LEFT_IN1 = 26;
constexpr uint8_t LEFT_IN2 = 27;
constexpr uint8_t LEFT_EN  = 25;

// Right Side
constexpr uint8_t RIGHT_IN1 = 33;
constexpr uint8_t RIGHT_IN2 = 32;
constexpr uint8_t RIGHT_EN  = 14;

// ==========================
// Probe Motor Driver (L298N #2)
// ==========================

constexpr uint8_t PROBE_IN1 = 18;
constexpr uint8_t PROBE_IN2 = 19;
constexpr uint8_t PROBE_EN  = 23;
    // ==========================
   
    // ==========================
    // Sensors
    // ==========================
    constexpr uint8_t DHT_PIN = 4;

    // GPS UART2
    constexpr uint8_t GPS_RX = 16;
    constexpr uint8_t GPS_TX = 17;

    // I2C Bus
    constexpr uint8_t SDA_PIN = 21;
    constexpr uint8_t SCL_PIN = 22;

    // Status LED
    constexpr uint8_t STATUS_LED = 2;
}
