#pragma once

const char sensorJS[] PROGMEM = R"rawliteral(
function applyTelemetry(data) {
    if (!data) return;
    setValue("temp", data.t + " °C");
    setValue("humidity", data.h + " %");
    setValue("soilTemp", data.st + " °C");
    setValue("moisture", data.m + " %");
    setValue("ph", data.ph);
    setValue("ec", data.ec);
    setValue("nitrogen", data.n);
    setValue("phosphorus", data.p);
    setValue("potassium", data.k);
    setValue("latitude", data.gps ? data.lat : "--");
    setValue("longitude", data.gps ? data.lon : "--");
    setValue("satellites", data.sat);
    setValue("gpsSpeed", data.gs + " km/h");
    setValue("altitude", data.alt + " m");
    updateGPSMap(data.gps ? data.lat : "--", data.gps ? data.lon : "--");
    updateConnection("gpsStatus", data.gps ? "CONNECTED" : "SEARCHING");
    updateConnection("soilStatus", data.nv ? "CONNECTED" : "NO DATA");
    updateConnection("dhtStatus", data.dv ? "CONNECTED" : "NO DATA");
    updateConnection("wifiStatus", "ONLINE");
    updateConnection("roverStatus", data.run ? "MOVING" : "IDLE");
}
)rawliteral";
