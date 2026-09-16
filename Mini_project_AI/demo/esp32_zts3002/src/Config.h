#pragma once

#include <Arduino.h>

namespace Config {
// Update these values to match your Wi-Fi, your actual TTL-RS485 board wiring,
// and the actual plot ID you want the ESP32 to report to.
// The sensor manual confirms: slave 0x01, baud 4800, 8N1.
constexpr const char* WIFI_SSID = "A";
constexpr const char* WIFI_PASSWORD = "12346789";
constexpr const char* BACKEND_URL = "http://192.168.1.50:5000/api/sensor/reading";
constexpr const char* PLOT_ID = "P01";

constexpr uint8_t MODBUS_ADDRESS = 0x01;
constexpr uint32_t MODBUS_BAUD = 4800;
constexpr uint8_t MODBUS_RX_PIN = 16;  // Set to the actual TTL-RS485 RX pin on your board
constexpr uint8_t MODBUS_TX_PIN = 17;  // Set to the actual TTL-RS485 TX pin on your board
constexpr uint8_t RS485_DE_PIN = 4;    // Set to DE pin on your TTL-RS485 converter
constexpr uint8_t RS485_RE_PIN = 5;    // Set to RE pin on your TTL-RS485 converter

constexpr uint32_t SENSOR_INTERVAL_MS = 30000UL;
constexpr uint32_t WIFI_TIMEOUT_MS = 15000UL;
}
