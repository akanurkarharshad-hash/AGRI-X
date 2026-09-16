#pragma once

const char controlJS[] PROGMEM = R"rawliteral(

// =====================================
// AGRI-X Rover Control Module
// Version : 1.0
// =====================================

// -----------------------------
// Rover Controls
// -----------------------------

function forward()
{
    api("/forward");
    addLog("Forward");
}

function backward()
{
    api("/backward");
    addLog("Backward");
}

function left()
{
    api("/left");
    addLog("Left");
}

function right()
{
    api("/right");
    addLog("Right");
}

function stop()
{
    api("/stop");
    addLog("Stop");
}

// -----------------------------
// Probe Controls
// -----------------------------

function probeInsert()
{
    api("/probe_insert");
    addLog("Probe Insert");
}

function probeRetract()
{
    api("/probe_retract");
    addLog("Probe Retract");
}

function probeStop()
{
    api("/probe_stop");
    addLog("Probe Stop");
}

// -----------------------------
// Emergency Stop
// -----------------------------

function emergencyStop()
{
    api("/stop");
    api("/probe_stop");

    addLog("Emergency Stop");

    alert("Emergency Stop Activated");
}

// -----------------------------
// Speed Slider
// -----------------------------

function updateSpeed(value)
{
    setValue("speedValue", value);

    api("/speed?value=" + value);

    addLog("Speed : " + value);
}

// =====================================
// Keyboard Controls (W A S D)
// =====================================

document.addEventListener("keydown", function (e) {

    // Prevent repeating while key is held
    if (e.repeat) return;

    switch (e.key.toLowerCase()) {

        case "w":
            forward();
            break;

        case "a":
            left();
            break;

        case "s":
            backward();
            break;

        case "d":
            right();
            break;

        case " ":
            e.preventDefault();
            stop();
            break;
    }
});

document.addEventListener("keyup", function (e) {

    switch (e.key.toLowerCase()) {

        case "w":
        case "a":
        case "s":
        case "d":
            stop();
            break;
    }
});

)rawliteral";

