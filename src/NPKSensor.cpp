#include "NPKSensor.h"
#define RS485_DIR_PIN 21

bool NPKSensor::begin()
{
    pinMode(RS485_DIR_PIN, OUTPUT);

    // Receive Mode
    digitalWrite(RS485_DIR_PIN, LOW);

    // UART1
    // RX = GPIO18
    // TX = GPIO19

    rs485.begin(4800, SERIAL_8N1, 18, 19);

    Serial.println("7-in-1 Soil Sensor Initialized");

    return true;
}

void NPKSensor::update()
{
    if (millis() - lastRead < 3000)
        return;

    lastRead = millis();

    readSensor();
}

bool NPKSensor::readSensor()
{
    uint8_t request[8];

    request[0] = 0x01;
    request[1] = 0x03;
    request[2] = 0x00;
    request[3] = 0x00;
    request[4] = 0x00;
    request[5] = 0x07;

    uint16_t crc = calculateCRC(request, 6);

    request[6] = crc & 0xFF;
    request[7] = crc >> 8;

    while (rs485.available())
    rs485.read();

//------------------------------
// Switch to TRANSMIT
//------------------------------

digitalWrite(RS485_DIR_PIN, HIGH);

delay(2);

rs485.write(request, 8);

rs485.flush();

delay(2);

//------------------------------
// Switch to RECEIVE
//------------------------------

digitalWrite(RS485_DIR_PIN, LOW);

delay(5);
    uint8_t response[32];

    uint8_t index = 0;

    unsigned long start = millis();

    while (millis() - start < 2000)
    {
        while (rs485.available())
        {
            response[index++] = rs485.read();

            if (index >= sizeof(response))
                break;
        }

        if (index >= 19)
            break;
    }

   if (index < 19)
{
    Serial.print("Received only ");
    Serial.print(index);
    Serial.println(" bytes");

    return false;
}
    Serial.print("Received ");
    Serial.print(index);
    Serial.println(" bytes");
    Serial.print("RAW : ");

for (uint8_t i = 0; i < index; i++)
{
    if (response[i] < 16)
        Serial.print("0");

    Serial.print(response[i], HEX);
    Serial.print(" ");
}

Serial.println();

    moisture = (response[3] << 8 | response[4]) / 10.0;

    temperature = (response[5] << 8 | response[6]) / 10.0;

    ec = (response[7] << 8 | response[8]);

    ph = (response[9] << 8 | response[10]) / 10.0;

    nitrogen = (response[11] << 8 | response[12]);

    phosphorus = (response[13] << 8 | response[14]);

    potassium = (response[15] << 8 | response[16]);

    return true;
}

float NPKSensor::getMoisture() const
{
    return moisture;
}

float NPKSensor::getTemperature() const
{
    return temperature;
}

float NPKSensor::getPH() const
{
    return ph;
}

float NPKSensor::getEC() const
{
    return ec;
}

uint16_t NPKSensor::getNitrogen() const
{
    return nitrogen;
}

uint16_t NPKSensor::getPhosphorus() const
{
    return phosphorus;
}

uint16_t NPKSensor::getPotassium() const
{
    return potassium;
}
uint16_t NPKSensor::calculateCRC(uint8_t *data, uint8_t length)
{
    uint16_t crc = 0xFFFF;

    for (uint8_t pos = 0; pos < length; pos++)
    {
        crc ^= (uint16_t)data[pos];

        for (uint8_t i = 0; i < 8; i++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}