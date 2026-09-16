// RS485 Manager for ESP32
// Controls DE (Driver Enable) and RE (Receiver Enable) pins of a
// half-duplex TTL-to-RS485 converter (e.g. MAX485).
//
// Direction control:
//   Transmit -> DE HIGH, RE HIGH
//   Receive  -> DE LOW,  RE LOW
//
// IMPORTANT: This class only toggles the direction pins. Completion of the
// UART transmission is the caller's responsibility and MUST be guaranteed by
// calling HardwareSerial::flush() (which blocks until the last stop bit has
// physically left the ESP32 via uart_wait_tx_done) BEFORE enableReceive().
// We therefore do NOT use arbitrary delays to "wait out" the transmission.
// Only a short transceiver line-settle is applied so the driver/receiver
// output has stabilised before data is clocked.

#ifndef AGRIX_RS485_H
#define AGRIX_RS485_H

#include "Arduino.h"

// Transceiver output settle time (line direction switch). This is a physical
// property of the RS485 driver, not a substitute for UART flush().
#define RS485_SETTLE_US 50

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

        // Start in receive mode (driver disabled, receiver enabled)
        digitalWrite(de_pin, LOW);
        digitalWrite(re_pin, LOW);
        is_transmitting = false;
    }

    // Switch the transceiver to transmit mode. Call this BEFORE writing bytes.
    void enableTransmit() {
        digitalWrite(de_pin, HIGH);  // Enable driver
        digitalWrite(re_pin, HIGH);  // Disable receiver
        is_transmitting = true;
        delayMicroseconds(RS485_SETTLE_US);  // let driver output settle
    }

    // Switch the transceiver to receive mode.
    // PRECONDITION: the caller has already called serial->flush() so the UART
    // transmission is fully complete. No transmission-length delay is needed.
    void enableReceive() {
        digitalWrite(de_pin, LOW);   // Disable driver
        digitalWrite(re_pin, LOW);   // Enable receiver
        is_transmitting = false;
        delayMicroseconds(RS485_SETTLE_US);  // let receiver input settle
    }

    bool isTransmitting() {
        return is_transmitting;
    }
};

#endif // AGRIX_RS485_H
