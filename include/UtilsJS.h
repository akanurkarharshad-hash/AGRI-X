#pragma once

const char utilsJS[] PROGMEM = R"rawliteral(

// ===============================
// AGRI-X Utility Library
// Version : 1.0
// ===============================

// Generic API Call
function api(url)
{
    return fetch(url)
        .catch(error =>
        {
            console.error("API Error:", error);
            addLog("Connection Error");
        });
}

// Safe JSON Fetch
async function fetchJSON(url)
{
    try
    {
        const response = await fetch(url);

        if(!response.ok)
            throw new Error("HTTP " + response.status);

        return await response.json();
    }
    catch(error)
    {
        console.error(error);
        addLog("Failed : " + url);
        return null;
    }
}

// Set element text safely
function setValue(id,value)
{
    const element=document.getElementById(id);

    if(element)
        element.innerHTML=value;
}

// Current Time
function getTime()
{
    const now=new Date();

    return now.toLocaleTimeString();
}

// Logger helper


)rawliteral";