#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "Config.h"
#include "SensorData.h"

HardwareSerial modbusSerial(2);

struct ModbusFrame {
  uint8_t address;
  uint8_t functionCode;
  uint16_t startAddress;
  uint16_t quantity;
  uint16_t crc;
};

uint16_t crc16Modbus(const uint8_t* data, size_t length) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; ++i) {
    crc ^= static_cast<uint16_t>(data[i]);
    for (uint8_t bit = 0; bit < 8; ++bit) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc = (crc >> 1);
      }
    }
  }
  return crc;
}

uint16_t crc16ModbusAppend(const uint8_t* data, size_t length) {
  uint16_t crc = crc16Modbus(data, length);
  uint8_t low = static_cast<uint8_t>(crc & 0xFF);
  uint8_t high = static_cast<uint8_t>((crc >> 8) & 0xFF);
  return (static_cast<uint16_t>(high) << 8) | static_cast<uint16_t>(low);
}

bool verifyCrc(const uint8_t* frame, size_t length) {
  if (length < 2) {
    return false;
  }
  uint16_t expected = static_cast<uint16_t>(frame[length - 2] << 8) | frame[length - 1];
  uint16_t actual = crc16Modbus(frame, length - 2);
  return expected == actual;
}

int16_t decodeSigned16(uint16_t raw) {
  return static_cast<int16_t>(raw);
}

bool sendModbusRequest(const uint8_t address, const uint16_t startRegister, const uint16_t quantity, uint8_t* responseBuffer, size_t& responseLength) {
  uint8_t request[8] = {
    address,
    0x03,
    static_cast<uint8_t>((startRegister >> 8) & 0xFF),
    static_cast<uint8_t>(startRegister & 0xFF),
    static_cast<uint8_t>((quantity >> 8) & 0xFF),
    static_cast<uint8_t>(quantity & 0xFF),
  };

  uint16_t crc = crc16Modbus(request, sizeof(request));
  request[6] = static_cast<uint8_t>(crc & 0xFF);
  request[7] = static_cast<uint8_t>((crc >> 8) & 0xFF);

  digitalWrite(Config::RS485_DE_PIN, HIGH);
  digitalWrite(Config::RS485_RE_PIN, HIGH);
  delay(10);
  modbusSerial.write(request, sizeof(request));
  modbusSerial.flush();
  delay(50);

  digitalWrite(Config::RS485_DE_PIN, LOW);
  digitalWrite(Config::RS485_RE_PIN, LOW);

  uint32_t start = millis();
  responseLength = 0;
  while (millis() - start < 500UL) {
    while (modbusSerial.available() > 0) {
      responseBuffer[responseLength++] = static_cast<uint8_t>(modbusSerial.read());
      if (responseLength >= 256) {
        break;
      }
    }
    if (responseLength > 0) {
      break;
    }
  }

  if (responseLength == 0) {
    return false;
  }

  return verifyCrc(responseBuffer, responseLength);
}

bool connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);

  uint32_t start = millis();
  Serial.printf("[WiFi] Connecting to %s\n", Config::WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED && millis() - start < Config::WIFI_TIMEOUT_MS) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFi] Connected. IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  }

  Serial.println("[WiFi] Connection failed.");
  return false;
}

bool sendReadingToBackend(const SensorData& data) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] Wi-Fi not connected.");
    return false;
  }

  StaticJsonDocument<512> payload;
  payload["plot_id"] = Config::PLOT_ID;
  payload["nitrogen"] = data.nitrogen;
  payload["phosphorus"] = data.phosphorus;
  payload["potassium"] = data.potassium;
  payload["ph"] = data.ph;

  String body;
  serializeJson(payload, body);

  HTTPClient http;
  http.begin(Config::BACKEND_URL);
  http.addHeader("Content-Type", "application/json");

  Serial.println("[HTTP] Sending reading to backend...");
  int httpCode = http.POST(body);
  String response = http.getString();

  Serial.printf("[HTTP] Response code: %d\n", httpCode);
  Serial.printf("[Backend] Response: %s\n", response.c_str());

  http.end();
  return httpCode >= 200 && httpCode < 300;
}

bool readDiagnosticPayload(SensorData& data) {
  uint8_t response[64] = {0};
  size_t responseLength = 0;

  Serial.println("[Modbus] Sending documented diagnostic query: 01 03 00 00 00 04 44 09");
  if (!sendModbusRequest(Config::MODBUS_ADDRESS, 0x0000, 0x0004, response, responseLength)) {
    data.error = "No valid Modbus RTU response received from the sensor.";
    Serial.println("[Modbus] No valid response or CRC error.");
    return false;
  }

  if (responseLength < 9) {
    data.error = "Unexpected Modbus response length.";
    Serial.println("[Modbus] Response length too short.");
    return false;
  }

  Serial.print("[Modbus] Raw bytes: ");
  for (size_t i = 0; i < responseLength; ++i) {
    Serial.printf("%02X ", response[i]);
  }
  Serial.println();

  uint8_t byteCount = response[2];
  if (byteCount != 8) {
    data.error = "Unexpected byte count in Modbus response.";
    Serial.printf("[Modbus] Byte count mismatch: %d\n", byteCount);
    return false;
  }

  uint16_t moistureRaw = (static_cast<uint16_t>(response[3]) << 8) | response[4];
  uint16_t tempRaw = (static_cast<uint16_t>(response[5]) << 8) | response[6];
  uint16_t conductivityRaw = (static_cast<uint16_t>(response[7]) << 8) | response[8];
  uint16_t phRaw = (static_cast<uint16_t>(response[9]) << 8) | response[10];

  data.moisture = static_cast<float>(decodeSigned16(moistureRaw)) / 10.0F;
  data.temperature = static_cast<float>(decodeSigned16(tempRaw)) / 10.0F;
  data.conductivity = static_cast<float>(conductivityRaw);
  data.ph = static_cast<float>(phRaw) / 10.0F;

  data.connected = true;
  data.valid = true;
  data.has_verified_ph = true;
  data.status = "diagnostic_ok";

  Serial.printf("[Sensor] Moisture: %.1f %%\n", data.moisture);
  Serial.printf("[Sensor] Temperature: %.1f C\n", data.temperature);
  Serial.printf("[Sensor] EC: %.1f uS/cm\n", data.conductivity);
  Serial.printf("[Sensor] pH: %.1f\n", data.ph);

  return true;
}

bool readNpkDiagnostics(SensorData& data) {
  // The supplied manual identifies registers 0x0004..0x0006 as temporary values and not as verified live NPK measurement registers.
  // This diagnostic probe is intentionally conservative: we log the evidence but do not fabricate a final NPK value.
  uint8_t response[64] = {0};
  size_t responseLength = 0;

  Serial.println("[Modbus] Probing temporary NPK registers 0x0004..0x0006 for evidence only.");
  if (!sendModbusRequest(Config::MODBUS_ADDRESS, 0x0004, 0x0003, response, responseLength)) {
    data.error = "Unable to read temporary NPK registers.";
    Serial.println("[Modbus] Temporary NPK register read failed.");
    return false;
  }

  Serial.print("[Modbus] Temporary NPK raw bytes: ");
  for (size_t i = 0; i < responseLength; ++i) {
    Serial.printf("%02X ", response[i]);
  }
  Serial.println();

  if (responseLength < 11) {
    Serial.println("[Modbus] Temporary NPK response is incomplete.");
    data.error = "Temporary NPK response incomplete.";
    return false;
  }

  Serial.println("[Sensor] Temporary values are not treated as final NPK measurement data until the manual register mapping is confirmed.");
  data.has_verified_npk = false;
  data.nitrogen = NAN;
  data.phosphorus = NAN;
  data.potassium = NAN;

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(Config::RS485_DE_PIN, OUTPUT);
  pinMode(Config::RS485_RE_PIN, OUTPUT);
  digitalWrite(Config::RS485_DE_PIN, LOW);
  digitalWrite(Config::RS485_RE_PIN, LOW);

  modbusSerial.begin(Config::MODBUS_BAUD, SERIAL_8N1, Config::MODBUS_RX_PIN, Config::MODBUS_TX_PIN);
  Serial.println("[System] ESP32 Modbus diagnostic firmware started.");

  if (!connectWiFi()) {
    Serial.println("[WiFi] Continuing in offline mode; backend will retry later.");
  }
}

void loop() {
  static uint32_t lastRun = 0;
  if (millis() - lastRun < Config::SENSOR_INTERVAL_MS) {
    delay(250);
    return;
  }
  lastRun = millis();

  SensorData sensor;
  sensor.status = "reading";

  bool ok = readDiagnosticPayload(sensor);
  if (!ok) {
    Serial.printf("[Error] %s\n", sensor.error.c_str());
    readNpkDiagnostics(sensor);
    delay(1000);
    return;
  }

  bool npkProbe = readNpkDiagnostics(sensor);
  if (npkProbe) {
    Serial.println("[Sensor] Temporary register evidence captured; final NPK values remain unverified until manufacturer mapping is confirmed.");
  }

  if (WiFi.status() == WL_CONNECTED) {
    bool sent = sendReadingToBackend(sensor);
    if (sent) {
      Serial.println("[Backend] Reading accepted.");
    } else {
      Serial.println("[Backend] Failed to send reading; will retry on next cycle.");
    }
  }

  Serial.println("[System] Sensor cycle complete.");
}
