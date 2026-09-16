// Sensor Manager for ESP32
// Orchestrates Modbus RTU communication with the ZTS-3002-TR-THNPKPH-N01 sensor.
// Reads 7 holding registers (Moisture, Temperature, EC, pH, N, P, K) in one
// transaction, validates the full frame (slave, function, byte count, length,
// CRC16, timeout, partial/malformed), and retains the last VALID reading so a
// failed poll never replaces good data with zeros.

#ifndef AGRIX_SENSOR_MANAGER_H
#define AGRIX_SENSOR_MANAGER_H

#include "Arduino.h"
#include "config.h"
#include "modbus_rtu.h"
#include "rs485_manager.h"

struct SensorData {
    ModbusRTU::SensorReading reading;   // Last VALID reading (retained across failures)
    bool has_valid_reading;             // True once at least one good read has occurred
    bool last_poll_ok;                  // Status of the most recent poll attempt
    unsigned long last_update_ms;       // millis() of the last VALID reading
    int error_count;                    // Consecutive failed polls
    String last_error;                  // Human-readable last error
};

class SensorManager {
private:
    RS485Manager* rs485;
    HardwareSerial* serial;
    SensorData sensor_data;

    // Mark the current poll as failed without disturbing the last valid reading.
    bool fail(const char* reason) {
        sensor_data.last_poll_ok = false;
        sensor_data.error_count++;
        sensor_data.last_error = reason;
        DEBUG_PRINTF("[AGRI-X][ERROR] %s\n", reason);
        return false;
    }

public:
    SensorManager(RS485Manager* rs485_mgr, HardwareSerial* serial_port)
        : rs485(rs485_mgr), serial(serial_port) {
        sensor_data.has_valid_reading = false;
        sensor_data.last_poll_ok = false;
        sensor_data.last_update_ms = 0;
        sensor_data.error_count = 0;
        sensor_data.last_error = "";
        // Zero the retained reading until the first successful poll.
        sensor_data.reading = ModbusRTU::SensorReading{};
    }

    void begin() {
        // 4800 baud, 8 data bits, no parity, 1 stop bit (8N1)
        serial->begin(MODBUS_BAUD_RATE, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
        DEBUG_PRINTLN("[AGRI-X] Modbus UART initialized");
        DEBUG_PRINTF("[AGRI-X] Baud: %d, RX: %d, TX: %d\n",
                     MODBUS_BAUD_RATE, RS485_RX_PIN, RS485_TX_PIN);
    }

    // Perform one Modbus poll. Returns true on a fully validated reading.
    bool pollSensor() {
        DEBUG_PRINTLN("[AGRI-X] Sensor polling...");

        // Build request: 01 03 00 00 00 07 <CRC_LO> <CRC_HI>
        uint8_t request[8];
        int request_len = ModbusRTU::buildReadRequest(
            request, SENSOR_SLAVE_ADDRESS, REG_MOISTURE, NUM_REGISTERS_TO_READ);
        if (request_len < 0) {
            return fail("Failed to build request");
        }

        // --- Transmit ---
        rs485->enableTransmit();
        while (serial->available()) serial->read();   // flush stale RX
        for (int i = 0; i < request_len; i++) serial->write(request[i]);
        serial->flush();                               // block until TX complete
        rs485->enableReceive();
        DEBUG_PRINTF("[AGRI-X] Sent %d bytes to sensor\n", request_len);

        // --- Receive with timeout and early frame-length detection ---
        // Expected valid frame for 7 registers:
        //   1 slave + 1 func + 1 byte-count(=14) + 14 data + 2 CRC = 19 bytes
        uint8_t response[256];
        size_t response_idx = 0;
        size_t expected_length = 0;
        unsigned long deadline = millis() + SENSOR_REQUEST_TIMEOUT_MS;

        while (millis() < deadline && response_idx < sizeof(response)) {
            if (!serial->available()) continue;
            response[response_idx++] = serial->read();

            // Once the 3-byte header is in, work out the total frame length.
            if (response_idx == 3 && expected_length == 0) {
                uint8_t func = response[1];
                if (func == MODBUS_FC_READ_HOLDING) {
                    expected_length = 3 + response[2] + MODBUS_CRC_LEN;  // header+data+CRC
                    if (expected_length > sizeof(response)) {
                        return fail("Response too large");
                    }
                } else if (func & 0x80) {
                    expected_length = 5;  // Modbus exception frame
                }
            }

            if (expected_length > 0 && response_idx >= expected_length) break;
        }

        // --- Frame-level validation ---
        if (response_idx == 0) {
            return fail("No response from sensor (timeout)");
        }
        if (expected_length == 0 || response_idx != expected_length) {
            DEBUG_PRINTF("[AGRI-X] got %d bytes, expected %d\n",
                         (int)response_idx, (int)expected_length);
            return fail("Incomplete/malformed response");
        }
        if (response[0] != SENSOR_SLAVE_ADDRESS) {
            return fail("Wrong slave address");
        }
        if (response[1] != MODBUS_FC_READ_HOLDING) {
            return fail("Wrong/exception function code");
        }
        if (response[2] != NUM_REGISTERS_TO_READ * 2) {   // must be 14
            return fail("Wrong byte count (expected 14)");
        }
        if (!ModbusCRC::verify(response, response_idx)) {
            return fail("CRC verification failed");
        }

        // --- Parse + convert ---
        uint16_t registers[NUM_REGISTERS_TO_READ];
        int num_regs = ModbusRTU::parseReadResponse(
            response, response_idx, registers, NUM_REGISTERS_TO_READ);
        if (num_regs < NUM_REGISTERS_TO_READ) {
            return fail("Failed to parse response");
        }

        ModbusRTU::SensorReading fresh;
        if (!ModbusRTU::convertRegisters(registers, num_regs, fresh)) {
            return fail("Failed to convert registers");
        }

        // --- Commit as the last valid reading ---
        sensor_data.reading = fresh;
        sensor_data.has_valid_reading = true;
        sensor_data.last_poll_ok = true;
        sensor_data.last_update_ms = millis();
        sensor_data.error_count = 0;
        sensor_data.last_error = "";

        DEBUG_PRINTF("[AGRI-X] Sensor OK  M:%.1f%% T:%.1fC EC:%.0f pH:%.1f N:%u P:%u K:%u\n",
                     fresh.moisture, fresh.temperature, fresh.ec, fresh.ph,
                     fresh.nitrogen, fresh.phosphorus, fresh.potassium);
        return true;
    }

    // Full sensor state (includes the retained last-valid reading).
    SensorData getLastReading() {
        return sensor_data;
    }

    // JSON payload for the backend. Field names MUST match the Flask endpoint
    // /api/plots/<plot_id>/readings/sensor (validate_sensor_reading_payload):
    // plot_id, device_id, moisture, temperature, ec, ph, nitrogen, phosphorus, potassium.
    String getReadingJSON() {
        if (!sensor_data.has_valid_reading) {
            return "{\"error\":\"No valid reading\"}";
        }
        const ModbusRTU::SensorReading& r = sensor_data.reading;
        String json = "{";
        json += "\"plot_id\":\"" + String(PLOT_ID) + "\",";
        json += "\"device_id\":\"" + String(DEVICE_ID) + "\",";
        json += "\"moisture\":" + String(r.moisture, 1) + ",";
        json += "\"temperature\":" + String(r.temperature, 1) + ",";
        json += "\"ec\":" + String(r.ec, 0) + ",";
        json += "\"ph\":" + String(r.ph, 1) + ",";
        json += "\"nitrogen\":" + String(r.nitrogen) + ",";
        json += "\"phosphorus\":" + String(r.phosphorus) + ",";
        json += "\"potassium\":" + String(r.potassium);
        json += "}";
        return json;
    }

    bool isHealthy() {
        return sensor_data.last_poll_ok && sensor_data.error_count < SENSOR_TIMEOUT_THRESHOLD;
    }

    // "OK"   = last poll succeeded
    // "STALE"= last poll failed but a previous valid reading is retained
    // "FAIL" = no valid data / too many consecutive errors
    String getStatus() {
        if (sensor_data.last_poll_ok) return "OK";
        if (sensor_data.has_valid_reading &&
            sensor_data.error_count < SENSOR_TIMEOUT_THRESHOLD) return "STALE";
        return "FAIL";
    }
};

#endif // AGRIX_SENSOR_MANAGER_H
