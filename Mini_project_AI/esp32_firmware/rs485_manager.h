// RS485 Manager for ESP32
// Controls DE (Driver Enable) and RE (Receiver Enable) pins
// Manages half-duplex TTL-to-RS485 converter

#ifndef AGRIX_RS485_H
#define AGRIX_RS485_H

#include "Arduino.h"

class RS485Manager {
private:
    uint8_t de_pin;
    uint8_t re_pin;
    bool is_transmitting;
    
public:
    RS485Manager(uint8_t de_pin, uint8_t re_pin)
        : de_pin(de_pin), re_pin(re_pin), is_transmitting(false) {}
    
    void begin() {
        pinMode(de_pin, OUTPUT);
        pinMode(re_pin, OUTPUT);
        
        // Initialize to receive mode
        digitalWrite(de_pin, LOW);   // Driver disabled
        digitalWrite(re_pin, LOW);   // Receiver enabled
        is_transmitting = false;
    }
    
    // Enable transmit mode
    void enableTransmit() {
        if (!is_transmitting) {
            digitalWrite(de_pin, HIGH);  // Enable driver
            digitalWrite(re_pin, HIGH);  // Disable receiver
            is_transmitting = true;
            
            // Small delay to ensure line is stable
            delayMicroseconds(100);
        }
    }
    
    // Enable receive mode
    void enableReceive() {
        if (is_transmitting) {
            // Wait for any transmission to complete
            delayMicroseconds(500);
            
            digitalWrite(de_pin, LOW);   // Disable driver
            digitalWrite(re_pin, LOW);   // Enable receiver
            is_transmitting = false;
            
            // Small delay to ensure stable
            delayMicroseconds(100);
        }
    }
    
    // Check if currently in transmit mode
    bool isTransmitting() {
        return is_transmitting;
    }
};

#endif // AGRIX_RS485_H
