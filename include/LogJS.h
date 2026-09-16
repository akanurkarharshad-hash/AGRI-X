#pragma once

const char logJS[] PROGMEM = R"rawliteral(

// =====================================
// AGRI-X Mission Log Module
// Version : 1.0
// =====================================

const MAX_LOGS = 50;

let missionLogs = [];

// ---------------------------------
// Add New Log
// ---------------------------------

function addLog(message)
{
    const time = getTime();

    missionLogs.unshift(
        "[" + time + "] " + message
    );

    if(missionLogs.length > MAX_LOGS)
        missionLogs.pop();

    renderLogs();
}

// ---------------------------------
// Render Logs
// ---------------------------------

function renderLogs()
{
    const log = document.getElementById("systemLogs");

    if(!log)
        return;

    log.innerHTML = missionLogs.join("<br>");
}

// ---------------------------------
// Clear Logs
// ---------------------------------

function clearLogs()
{
    missionLogs = [];
    renderLogs();
}

// ---------------------------------
// Export Logs (Future)
// ---------------------------------

function exportLogs()
{
    console.log("Export Logs");
}

)rawliteral";