#pragma once

const char sensorJS[] PROGMEM = R"rawliteral(

// =====================================
// AGRI-X Sensor Module
// Version : 1.0
// =====================================

async function updateSensors()
{
    const data = await fetchJSON("/sensorData");

    if(data == null)
        return;

    // --------------------------
    // Air Sensor
    // --------------------------

    setValue("temp", data.temperature + " °C");
    setValue("humidity", data.humidity + " %");

    // --------------------------
    // Soil Sensor
    // --------------------------

    setValue("soilTemp", data.soilTemperature + " °C");
    setValue("moisture", data.moisture + " %");
    setValue("ph", data.ph);
    setValue("ec", data.ec);

    setValue("nitrogen", data.nitrogen);
    setValue("phosphorus", data.phosphorus);
    setValue("potassium", data.potassium);

    // --------------------------
    // GPS
    // --------------------------

    setValue("latitude", data.latitude);
    setValue("longitude", data.longitude);
    setValue("satellites", data.satellites);
    setValue("gpsSpeed", data.speed + " km/h");
    setValue("altitude", data.altitude + " m");

    updateGPSMap(
    data.latitude,
    data.longitude
);

}
    updateSensors();
    setInterval(updateSensors, 2000);

)rawliteral";