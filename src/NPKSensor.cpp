#include "NPKSensor.h"
#include "TimingConfig.h"

#define RS485_DIR_PIN 21

bool NPKSensor::begin()
{
    pinMode(RS485_DIR_PIN, OUTPUT);
    digitalWrite(RS485_DIR_PIN, LOW);
    rs485.begin(4800, SERIAL_8N1, 18, 19);
    return true;
}

void NPKSensor::update()
{
    const uint32_t now = millis();
    if (!requestPending && (now - lastRead >= TimingConfig::NPK_SAMPLE_INTERVAL_MS))
    {
        startRequest(now);
        return;
    }
    if (!requestPending)
        return;

    while (rs485.available() && responseLength < sizeof(response))
        response[responseLength++] = static_cast<uint8_t>(rs485.read());

    if (responseLength >= 19)
    {
        finishResponse();
        return;
    }
    if (now - requestStarted >= TimingConfig::NPK_RESPONSE_TIMEOUT_MS)
    {
        requestPending = false;
        valid = false;
        lastRead = now;
    }
}

void NPKSensor::startRequest(uint32_t now)
{
    uint8_t request[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0, 0};
    const uint16_t crc = calculateCRC(request, 6);
    request[6] = crc & 0xFF;
    request[7] = crc >> 8;
    while (rs485.available())
        rs485.read();

    digitalWrite(RS485_DIR_PIN, HIGH);
    rs485.write(request, sizeof(request));
    // Called from the low-priority sensor task; RS485 must finish transmitting
    // before DE is lowered, while the Arduino loop and drive watchdog keep running.
    rs485.flush();
    digitalWrite(RS485_DIR_PIN, LOW);

    responseLength = 0;
    requestStarted = now;
    lastRead = now;
    requestPending = true;
}

void NPKSensor::finishResponse()
{
    const uint16_t expected = calculateCRC(response, 17);
    const uint16_t received = static_cast<uint16_t>(response[17]) |
                              (static_cast<uint16_t>(response[18]) << 8);
    if (response[0] != 0x01 || response[1] != 0x03 || response[2] != 14 || expected != received)
    {
        valid = false;
        requestPending = false;
        return;
    }

    moisture = (response[3] << 8 | response[4]) / 10.0f;
    temperature = (response[5] << 8 | response[6]) / 10.0f;
    ec = (response[7] << 8 | response[8]);
    ph = (response[9] << 8 | response[10]) / 10.0f;
    nitrogen = (response[11] << 8 | response[12]);
    phosphorus = (response[13] << 8 | response[14]);
    potassium = (response[15] << 8 | response[16]);
    valid = true;
    validTimestamp = millis();
    requestPending = false;
}

bool NPKSensor::isValid() const { return valid; }
uint32_t NPKSensor::getTimestamp() const { return validTimestamp; }
float NPKSensor::getMoisture() const { return moisture; }
float NPKSensor::getTemperature() const { return temperature; }
float NPKSensor::getPH() const { return ph; }
float NPKSensor::getEC() const { return ec; }
uint16_t NPKSensor::getNitrogen() const { return nitrogen; }
uint16_t NPKSensor::getPhosphorus() const { return phosphorus; }
uint16_t NPKSensor::getPotassium() const { return potassium; }

uint16_t NPKSensor::calculateCRC(const uint8_t *data, uint8_t length)
{
    uint16_t crc = 0xFFFF;
    for (uint8_t pos = 0; pos < length; ++pos)
    {
        crc ^= data[pos];
        for (uint8_t i = 0; i < 8; ++i)
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
    return crc;
}
