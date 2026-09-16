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
    // Verified against working NPKSensor.cpp/NPKSensor.h implementation
    struct SensorReading {
        float moisture;           // % (divided by 10)
        float temperature;        // °C (divided by 10, signed)
        float ec;                 // µS/cm (raw value)
        float ph;                 // pH units (divided by 10)
        uint16_t nitrogen;        // mg/kg (raw value)
        uint16_t phosphorus;      // mg/kg (raw value)
        uint16_t potassium;       // mg/kg (raw value)
    };
    
    static bool convertRegisters(
        const uint16_t* registers,
        size_t num_registers,
        SensorReading& reading
    ) {
        // Must have all 7 registers (Moisture, Temperature, EC, pH, N, P, K)
        if (num_registers < 7) return false;
        
        // Register values use documented scaling from ZTS-3002 manual
        // Verified against working NPKSensor.cpp/NPKSensor.h implementation
        
        // Register 0: Moisture (%)
        // Example: 0x0292 (658) -> 65.8%
        reading.moisture = registers[0] / 10.0f;
        
        // Register 1: Temperature (°C) - SIGNED 16-bit
        // Example: 0xFF9B (-101 signed) -> -10.1°C
        int16_t temp_raw = static_cast<int16_t>(registers[1]);
        reading.temperature = temp_raw / 10.0f;
        
        // Register 2: EC (µS/cm)
        // Example: 0x03E8 (1000) -> 1000 µS/cm (1:1 scale)
        reading.ec = static_cast<float>(registers[2]);
        
        // Register 3: pH
        // Example: 0x0038 (56) -> 5.6 pH
        reading.ph = registers[3] / 10.0f;
        
        // Register 4: Nitrogen (mg/kg)
        // Raw value, no scaling
        reading.nitrogen = registers[4];
        
        // Register 5: Phosphorus (mg/kg)
        // Raw value, no scaling
        reading.phosphorus = registers[5];
        
        // Register 6: Potassium (mg/kg)
        // Raw value, no scaling
        reading.potassium = registers[6];
        
        return true;
    }
};

#endif // AGRIX_MODBUS_H
