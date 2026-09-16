#pragma once

const char statusJS[] PROGMEM = R"rawliteral(

// =====================================
// AGRI-X Status Module
// Version : 1.0
// =====================================

async function updateStatus()
{
    const data = await fetchJSON("/status");

    if(data == null)
    {
        setOffline();
        return;
    }

    updateConnection("wifiStatus", data.wifi);
    updateConnection("gpsStatus", data.gps);
    updateConnection("soilStatus", data.soil);
    updateConnection("dhtStatus", data.dht);
    updateConnection("probeStatus", data.probe);
    updateConnection("roverStatus", data.rover);
}

// -----------------------------
// Update Connection Indicator
// -----------------------------

function updateConnection(id,status)
{
    const element=document.getElementById(id);

    if(!element)
        return;

    element.innerHTML=status;

    if(status=="ONLINE" ||
       status=="CONNECTED" ||
       status=="READY" ||
       status=="IDLE")
    {
        element.className="statusValue online";
    }
    else
    {
        element.className="statusValue offline";
    }
}

// -----------------------------
// ESP32 Offline
// -----------------------------

function setOffline()
{
    const ids=[
        "wifiStatus",
        "gpsStatus",
        "soilStatus",
        "dhtStatus",
        "probeStatus",
        "roverStatus"
    ];

    ids.forEach(id=>
    {
        const element=document.getElementById(id);

        if(element)
        {
            element.innerHTML="OFFLINE";
            element.className="statusValue offline";
        }
    });
}

)rawliteral";