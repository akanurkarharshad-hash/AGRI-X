#pragma once

#include <stdint.h>

// Central timing and access settings for the control and sensor paths.
namespace TimingConfig
{
    constexpr uint32_t DRIVE_WATCHDOG_MS = 400;
    constexpr uint32_t TELEMETRY_INTERVAL_MS = 500;
    constexpr uint32_t NPK_SAMPLE_INTERVAL_MS = 3000;
    constexpr uint32_t NPK_RESPONSE_TIMEOUT_MS = 250;
    constexpr uint32_t DHT_SAMPLE_INTERVAL_MS = 2000;
    constexpr uint32_t SENSOR_TASK_PERIOD_MS = 2;
    constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
    constexpr uint32_t WIFI_RESTART_DELAY_MS = 1000;
    constexpr uint8_t GPS_BYTES_PER_LOOP = 32;
    constexpr char WEBSOCKET_SHARED_KEY[] = "agrix-change-this-key";
    constexpr char DASHBOARD_DEFAULT_HOST[] = "192.168.4.1";
}
