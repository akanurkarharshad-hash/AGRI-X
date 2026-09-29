(function () {
    "use strict";
    var config = window.AGRIX_CONFIG || {};
    var host = config.ESP32_HOST || location.hostname;
    var ws = null, seq = 0, role = "pending", held = "", timer = 0;
    var speed = document.getElementById("speed");
    var output = document.querySelector("output");
    var status = document.getElementById("connection");

    function connect() {
        ws = new WebSocket("ws://" + host + "/ws?key=" + encodeURIComponent(config.WS_SHARED_KEY || ""));
        ws.onopen = function () { status.textContent = "Connected; waiting for controller"; };
        ws.onclose = function () { role = "offline"; stopRepeat(false); status.textContent = "Disconnected"; setTimeout(connect, 1000); };
        ws.onmessage = function (event) {
            if (event.data === "R,controller") { role = "controller"; status.textContent = "Controller active"; return; }
            if (event.data === "R,readonly") { role = "readonly"; status.textContent = "Read only: another controller is connected"; return; }
            try {
                var data = JSON.parse(event.data);
                Object.keys(data).forEach(function (key) {
                    var field = document.querySelector('[data-field="' + key + '"]');
                    if (field) field.textContent = key === "run" ? (data[key] ? "MOVING" : "STOPPED") : data[key];
                });
            } catch (_) { }
        };
    }
    function send(command) {
        if (role === "controller" && ws && ws.readyState === WebSocket.OPEN)
            ws.send("M," + (++seq) + "," + command + "," + speed.value);
    }
    function stop() {
        held = ""; if (timer) clearInterval(timer); timer = 0;
        if (ws && ws.readyState === WebSocket.OPEN) ws.send("S," + (++seq));
    }
    function stopRepeat(sendStop) { held = ""; if (timer) clearInterval(timer); timer = 0; if (sendStop) stop(); }
    function start(command) {
        stopRepeat(false); held = command; send(command);
        timer = setInterval(function () { if (held) send(held); }, 100);
    }
    document.querySelectorAll("[data-move]").forEach(function (button) {
        button.addEventListener("pointerdown", function (event) { event.preventDefault(); start(button.dataset.move); });
    });
    document.getElementById("stop").addEventListener("click", stop);
    document.addEventListener("pointerup", function () { if (held) stopRepeat(true); });
    document.addEventListener("pointercancel", function () { if (held) stopRepeat(true); });
    window.addEventListener("blur", function () { if (held) stopRepeat(true); });
    document.addEventListener("keydown", function (event) {
        if (event.repeat) return;
        var keys = { w: "F", a: "L", s: "B", d: "R" }, command = keys[event.key.toLowerCase()];
        if (command) start(command); else if (event.key === " ") { event.preventDefault(); stop(); }
    });
    document.addEventListener("keyup", function (event) {
        if (["w", "a", "s", "d"].indexOf(event.key.toLowerCase()) >= 0) stopRepeat(true);
    });
    speed.addEventListener("input", function () { output.value = speed.value; output.textContent = speed.value; });
    connect();
}());
