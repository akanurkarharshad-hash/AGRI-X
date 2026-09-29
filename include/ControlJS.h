#pragma once

const char controlJS[] PROGMEM = R"rawliteral(
// For a laptop-hosted copy, set ESP32_HOST to the rover IP. Embedded mode
// substitutes the current ESP32 hostname and the firmware shared key.
var ESP32_HOST = "%ESP32_HOST%";
var WS_SHARED_KEY = "%WS_KEY%";
var roverSocket = null;
var roverSequence = 0;
var roverSpeedValue = 180;
var heldCommand = "";
var holdTimer = null;
var controllerRole = "offline";

function socketAddress() {
    var host = ESP32_HOST || window.location.hostname;
    return "ws://" + host + "/ws?key=" + encodeURIComponent(WS_SHARED_KEY);
}

function connectRoverSocket() {
    roverSocket = new WebSocket(socketAddress());
    roverSocket.onopen = function() { controllerRole = "pending"; addLog("WebSocket connected; send a fresh command"); };
    roverSocket.onclose = function() {
        controllerRole = "offline";
        stopHold(false);
        setOffline();
        window.setTimeout(connectRoverSocket, 1000);
    };
    roverSocket.onerror = function() { controllerRole = "offline"; };
    roverSocket.onmessage = function(event) {
        if (event.data === "R,controller") { controllerRole = "controller"; return; }
        if (event.data === "R,readonly") { controllerRole = "readonly"; addLog("Connected read-only; another client controls rover"); return; }
        try { applyTelemetry(JSON.parse(event.data)); } catch (_) { }
    };
}

function sendMove(command) {
    if (!roverSocket || roverSocket.readyState !== WebSocket.OPEN || controllerRole !== "controller") return;
    roverSocket.send("M," + (++roverSequence) + "," + command + "," + roverSpeedValue);
}
function startHold(command) {
    stopHold(false);
    heldCommand = command;
    sendMove(command);
    holdTimer = window.setInterval(function() { if (heldCommand) sendMove(heldCommand); }, 100);
    addLog(command);
}
function stopHold(sendStop) {
    heldCommand = "";
    if (holdTimer) { window.clearInterval(holdTimer); holdTimer = null; }
    if (sendStop !== false) stop();
}
function forward() { startHold("F"); }
function backward() { startHold("B"); }
function left() { startHold("L"); }
function right() { startHold("R"); }
function stop() {
    heldCommand = "";
    if (holdTimer) { window.clearInterval(holdTimer); holdTimer = null; }
    if (roverSocket && roverSocket.readyState === WebSocket.OPEN)
        roverSocket.send("S," + (++roverSequence));
    addLog("Stop");
}
function emergencyStop() { stop(); api("/probe_stop"); addLog("Emergency Stop"); }

function updateSpeed(value) {
    roverSpeedValue = Math.max(0, Math.min(255, Number(value) || 0));
    setValue("speedValue", roverSpeedValue);
}

function probeInsert() { api("/probe_insert"); addLog("Probe Insert"); }
function probeRetract() { api("/probe_retract"); addLog("Probe Retract"); }
function probeStop() { api("/probe_stop"); addLog("Probe Stop"); }

document.addEventListener("pointerup", function() { if (heldCommand) stopHold(true); });
document.addEventListener("pointercancel", function() { if (heldCommand) stopHold(true); });
document.addEventListener("keydown", function(e) {
    if (e.repeat) return;
    switch (e.key.toLowerCase()) {
        case "w": forward(); break;
        case "a": left(); break;
        case "s": backward(); break;
        case "d": right(); break;
        case " ": e.preventDefault(); stop(); break;
    }
});
document.addEventListener("keyup", function(e) {
    if (["w", "a", "s", "d"].indexOf(e.key.toLowerCase()) >= 0) stopHold(true);
});
window.addEventListener("blur", function() { if (heldCommand) stopHold(true); });
window.addEventListener("load", connectRoverSocket);
)rawliteral";
