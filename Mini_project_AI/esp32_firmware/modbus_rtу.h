// Modbus RTU Driver for ESP32
// Handles CRC16, request generation, response parsing for ZTS-3002 sensor

#ifndef AGRIX_MODBUS_H
#define AGRIX_MODBUS_H

#include <stdint.h>
#include <string.h>

// CRC16-MODBUS implementation
class ModbusCRC {
public:
    static uint16_t calculate(const uint8_t* buffer, size_t length) {
        uint16_t crc = 0xFFFF;
        for (size_t i = 0; i < length; i++) {
            crc ^= buffer[i];
            for (int j = 0; j < 8; j++) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }
    
    static bool verify(const uint8_t* buffer, size_t length) {
        if (length < 2) return false;
        uint16_t crc_calc = calculate(buffer, length - 2);
        uint16_t crc_received = (buffer[length - 1] << 8) | buffer[length - 2];
        return crc_calc == crc_received;
    }
};

// Modbus RTU Request/Response Builder
class ModbusRTU {
public:
    // Build a Modbus read holding registers request
    // Returns length of request, or -1 on error
    static int buildReadRequest(
        uint8_t* buffer,
        uint8_t slave_address,
        uint16_t start_register,
        uint16_t num_registers
    ) {
        if (!buffer) return -1;
        
        // Request format:
        // [Slave Addr] [Function] [Start Addr H] [Start Addr L] 
        // [Num Regs H] [Num Regs L] [CRC L] [CRC H]
        
        buffer[0] = slave_address;
        buffer[1] = 0x03;  // Function: Read Holding Registers
        buffer[2] = (start_register >> 8) & 0xFF;
        buffer[3] = start_register & 0xFF;
        buffer[4] = (num_registers >> 8) & 0xFF;
        buffer[5] = num_registers & 0xFF;
        
        // Calculate CRC
        uint16_t crc = ModbusCRC::calculate(buffer, 6);
        buffer[6] = crc & 0xFF;        // CRC Low byte
        buffer[7] = (crc >> 8) & 0xFF; // CRC High byte
        
        return 8;  // Total request length
    }
    
    // Parse Modbus read response
    // Returns number of registers parsed, or -1 on error
    static int parseReadResponse(
        const uint8_t* buffer,
        size_t length,
        uint16_t* out_values,
        size_t max_values
    ) {
        if (!buffer || !out_values || max_values == 0) return -1;
        
        // Minimum response: [Slave] [Func] [Byte Count] [Data] [CRC2]
        if (length < 5) return -1;
        
        // Verify CRC
        if (!ModbusCRC::verify(buffer, length)) {
            return -1;  // CRC failed
        }
        
        uint8_t slave_addr = buffer[0];
        uint8_t function = buffer[1];
        uint8_t byte_count = buffer[2];
        
        // Validate response
        if (function != 0x03) return -1;
        if (byte_count % 2 != 0) return -1;  // Byte count must be even
        
        size_t num_registers = byte_count / 2;
        if (num_registers > max_values) num_registers = max_values;
        
        // Extract register values (big-endian)
        for (size_t i = 0; i < num_registers; i++) {
            uint8_t high = buffer[3 + (i * 2)];
            uint8_t low = buffer[4 + (i * 2)];
            out_values[i] = (high << 8) | low;
        }
        
        return (int)num_registers;
    }
    
    // Convert Modbus register value to sensor measurement
    // These conversions are from ZTS-3002 documentation
    struct SensorReading {
        float moisture;      // %
        float temperature;   // °C
        float ec;            // µS/cm
        float ph;            // pH units
    };
    
    static bool convertRegisters(
        const uint16_t* registers,
        size_t num_registers,
        SensorReading& reading
    ) {
        if (num_registers < 4) return false;
        
        // Register values use documented scaling
        // These conversions match the manual's examples:
        // Moisture: 0x0292 (658) -> 65.8% means divide by 10
        // Temperature: 0xFF9B (-101 signed) -> -10.1°C means divide by 10
        // EC: 0x03E8 (1000) -> 1000 µS/cm means 1:1
        // pH: 0x0038 (56) -> 5.6 means divide by 10
        
        // Moisture (register 0)
        reading.moisture = registers[0] / 10.0f;
        
        // Temperature (register 1) - signed 16-bit
        int16_t temp_raw = (int16_t)registers[1];
        reading.temperature = temp_raw / 10.0f;
        
        // EC (register 2)
        reading.ec = registers[2] * 1.0f;
        
        // pH (register 3)
        reading.ph = registers[3] / 10.0f;
        
        return true;
    }
};

#endif // AGRIX_MODBUS_H
