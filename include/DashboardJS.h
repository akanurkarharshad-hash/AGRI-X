#pragma once

const char dashboardJS[] PROGMEM = R"rawliteral(

// =====================================
// AGRI-X Dashboard Controller
// Version 1.0
// =====================================

// Initialize Dashboard
window.onload = function()
{
    updateSpeed(180);
    setInterval(function()
{
    const img = document.getElementById("liveCamera");

    if(img)
    {
        img.src =
        "http://172.21.36.75:8000/static/latest.jpg?t=" + Date.now();
    }

},200);

    addLog("Dashboard Started");
};

/*=========================
      LIVE CAMERA
=========================*/

function startCamera()
{
    const camera =
        document.getElementById("cameraFeed");

    if(!camera)
        return;

    camera.src =
        "http://172.21.36.75:8080/video";
}

window.addEventListener("load", startCamera);

// =====================================
// AGRI-X GPS MAP
// =====================================

let gpsMap = null;
let roverMarker = null;

function initGPSMap()
{
    const mapElement = document.getElementById("gpsMap");

    if (!mapElement)
        return;

    gpsMap = L.map("gpsMap").setView(
        [20.5937, 78.9629],
        5
    );

    L.tileLayer(
        "https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png",
        {
            maxZoom: 19,
            attribution: "&copy; OpenStreetMap contributors"
        }
    ).addTo(gpsMap);
}

function updateGPSMap(latitude, longitude)
{
    if (!gpsMap)
        return;

    if (
        latitude === "--" ||
        longitude === "--" ||
        latitude == null ||
        longitude == null
    )
    {
        return;
    }

    const lat = Number(latitude);
    const lon = Number(longitude);

    if (!Number.isFinite(lat) || !Number.isFinite(lon))
        return;

    const position = [lat, lon];

    if (!roverMarker)
    {
        roverMarker = L.marker(position)
            .addTo(gpsMap)
            .bindPopup("🚜 AGRI-X Rover");

        gpsMap.setView(position, 18);
    }
    else
    {
        roverMarker.setLatLng(position);
        gpsMap.panTo(position);
    }
}

window.addEventListener("load", function()
{
    initGPSMap();
});

)rawliteral";
