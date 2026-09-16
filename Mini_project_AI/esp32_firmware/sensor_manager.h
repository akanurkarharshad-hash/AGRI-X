// Sensor Manager for ESP32
// Orchestrates Modbus communication with ZTS-3002 sensor
// Handles timeouts, retries, and error logging

#ifndef AGRIX_SENSOR_MANAGER_H
#define AGRIX_SENSOR_MANAGER_H

#include "Arduino.h"
#include "config.h"
#include "modbus_rtу.h"
#include "rs485_manager.h"

struct SensorData {
    ModbusRTU::SensorReading reading;
    bool is_valid;
    unsigned long last_update_ms;
    int error_count;
    String last_error;
};

class SensorManager {
private:
    RS485Manager* rs485;
    HardwareSerial* serial;
    SensorData sensor_data;
    
public:
    SensorManager(RS485Manager* rs485_mgr, HardwareSerial* serial_port)
        : rs485(rs485_mgr), serial(serial_port) {
        sensor_data.is_valid = false;
        sensor_data.last_update_ms = 0;
        sensor_data.error_count = 0;
        sensor_data.last_error = "";
    }
    
    void begin() {
        // Initialize UART for Modbus
        serial->begin(
    MODBUS_BAUD_RATE,
    SERIAL_8N1,
    RS485_RX_PIN,
    RS485_TX_PIN
);
        
        DEBUG_PRINTLN("[AGRI-X] Modbus UART initialized");
        DEBUG_PRINTF("[AGRI-X] Baud: %d, RX: %d, TX: %d\n", 
                     MODBUS_BAUD_RATE, RS485_RX_PIN, RS485_TX_PIN);
    }
    
    // Poll sensor for current reading
    bool pollSensor() {
        DEBUG_PRINTLN("[AGRI-X] Sensor polling...");
        
        // Build Modbus request
        uint8_t request[8];
        int request_len = ModbusRTU::buildReadRequest(
            request,
            SENSOR_SLAVE_ADDRESS,
            REG_MOISTURE,
            NUM_REGISTERS_TO_READ
        );
        
        if (request_len < 0) {
            sensor_data.last_error = "Failed to build request";
            sensor_data.is_valid = false;
            sensor_data.error_count++;
            DEBUG_PRINTLN("[AGRI-X][ERROR] Modbus request build failed");
            return false;
        }
        
        // Transmit request
        rs485->enableTransmit();
        
        // Clear any pending data
        while (serial->available()) {
            serial->read();
        }
        
        // Send request
        for (int i = 0; i < request_len; i++) {
            serial->write(request[i]);
        }
        serial->flush();
        
        DEBUG_PRINTF("[AGRI-X] Sent %d bytes to sensor\n", request_len);
        
        // Switch to receive mode
        rs485->enableReceive();
        
        // Wait for response with timeout
        unsigned long timeout_ms = millis() + SENSOR_REQUEST_TIMEOUT_MS;
        uint8_t response[256];
        size_t response_idx = 0;
        
        while (millis() < timeout_ms && response_idx < sizeof(response)) {
            if (serial->available()) {
                response[response_idx++] = serial->read();
                
                // Minimum valid response is 9 bytes (for 4 registers)
                if (response_idx >= 9) {
                    // Check if we have a complete response
                    if (response[2] == 8) {  // Byte count for 4 registers
                        response_idx = 13;  // 1+1+1+8+2 = 13 bytes total (was incorrectly 11)
                        break;
                    }
                }
            }
        }
        
        if (response_idx == 0) {
            sensor_data.last_error = "No response from sensor";
            sensor_data.is_valid = false;
            sensor_data.error_count++;
            DEBUG_PRINTLN("[AGRI-X][ERROR] Sensor timeout - no response");
            return false;
        }
        
        DEBUG_PRINTF("[AGRI-X] Received %d bytes from sensor\n", response_idx);
        
        // Verify CRC
        if (!ModbusCRC::verify(response, response_idx)) {
            sensor_data.last_error = "CRC verification failed";
            sensor_data.is_valid = false;
            sensor_data.error_count++;
            DEBUG_PRINTLN("[AGRI-X][ERROR] Modbus CRC mismatch");
            return false;
        }
        
        // Parse response
        uint16_t registers[NUM_REGISTERS_TO_READ];
        int num_regs = ModbusRTU::parseReadResponse(
            response,
            response_idx,
            registers,
            NUM_REGISTERS_TO_READ
        );
        
        if (num_regs < NUM_REGISTERS_TO_READ) {
            sensor_data.last_error = "Failed to parse response";
            sensor_data.is_valid = false;
            sensor_data.error_count++;
            DEBUG_PRINTF("[AGRI-X][ERROR] Parse failed: got %d regs, expected %d\n", 
                        num_regs, NUM_REGISTERS_TO_READ);
            return false;
        }
        
        // Convert to sensor readings
        if (!ModbusRTU::convertRegisters(registers, num_regs, sensor_data.reading)) {
            sensor_data.last_error = "Failed to convert registers";
            sensor_data.is_valid = false;
            sensor_data.error_count++;
            DEBUG_PRINTLN("[AGRI-X][ERROR] Register conversion failed");
            return false;
        }
        
        // Success!
        sensor_data.is_valid = true;
        sensor_data.last_update_ms = millis();
        sensor_data.error_count = 0;
        sensor_data.last_error = "";
        
        DEBUG_PRINTF("[AGRI-X] Sensor OK\n");
        DEBUG_PRINTF("[AGRI-X]   Moisture: %.1f%%\n", sensor_data.reading.moisture);
        DEBUG_PRINTF("[AGRI-X]   Temperature: %.1f°C\n", sensor_data.reading.temperature);
        DEBUG_PRINTF("[AGRI-X]   EC: %.0fµS/cm\n", sensor_data.reading.ec);
        DEBUG_PRINTF("[AGRI-X]   pH: %.1f\n", sensor_data.reading.ph);
        
        return true;
    }
    
    // Get last valid reading
    SensorData getLastReading() {
        return sensor_data;
    }
    
    // Get sensor reading as JSON string
    String getReadingJSON() {
        if (!sensor_data.is_valid) {
            return "{\"error\":\"No valid reading\"}";
        }
        
        // Build JSON payload for backend API
        String json = "{";
        json += "\"plot_id\":\"" + String(PLOT_ID) + "\",";
        json += "\"device_id\":\"" + String(DEVICE_ID) + "\",";
        json += "\"moisture\":" + String(sensor_data.reading.moisture, 1) + ",";
        json += "\"temperature\":" + String(sensor_data.reading.temperature, 1) + ",";
        json += "\"ec\":" + String(sensor_data.reading.ec, 0) + ",";
        json += "\"ph\":" + String(sensor_data.reading.ph, 1);
        json += "}";
        
        return json;
    }
    
    bool isHealthy() {
        return sensor_data.is_valid && sensor_data.error_count < SENSOR_TIMEOUT_THRESHOLD;
    }
    
    String getStatus() {
        if (sensor_data.is_valid) {
            return "OK";
        } else if (sensor_data.error_count < SENSOR_TIMEOUT_THRESHOLD) {
            return "Retry";
        } else {
            return "FAIL";
        }
    }
};

#endif // AGRIX_SENSOR_MANAGER_H
