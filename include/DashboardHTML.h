#pragma once

#include "DashboardCSS.h"
#include "DashboardJS.h"

const char dashboardHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>AGRI-X Dashboard</title>

<style>
%CSS%
</style>

<link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css">

</head>

<body>

<div class="dashboard">

  <!-- ===================== HEADER ===================== -->
  <div class="topbar">

    <div class="brand">
      <div class="brand-icon">🚜</div>
      <div class="brand-text">
        <h1>AGRI-X</h1>
        <span>AI SMART AGRICULTURAL ROVER</span>
      </div>
    </div>

    <div class="statusRow">

      <div class="statusPill">
        <div>
          <span class="statusPill-label">ESP32</span>
          <span id="wifiStatus" class="statusValue online">ONLINE</span>
        </div>
      </div>

      <div class="statusPill">
        <div>
          <span class="statusPill-label">GPS</span>
          <span id="gpsStatus" class="statusValue online">CONNECTED</span>
        </div>
      </div>

      <div class="statusPill">
        <div>
          <span class="statusPill-label">SOIL SENSOR</span>
          <span id="soilStatus" class="statusValue online">CONNECTED</span>
        </div>
      </div>

      <div class="statusPill">
        <div>
          <span class="statusPill-label">DHT11</span>
          <span id="dhtStatus" class="statusValue online">CONNECTED</span>
        </div>
      </div>

      <div class="statusPill">
        <div>
          <span class="statusPill-label">PROBE</span>
          <span id="probeStatus" class="statusValue online">READY</span>
        </div>
      </div>

      <div class="statusPill">
        <div>
          <span class="statusPill-label">ROVER</span>
          <span id="roverStatus" class="statusValue online">IDLE</span>
        </div>
      </div>

      <button class="emergency" onclick="emergencyStop()">⛔ EMERGENCY STOP</button>

    </div>

    <div class="headerRight">
      <div class="clockBox">
        <div id="liveClock">--:--:--</div>
        <div class="clockLabel">SYSTEM TIME</div>
      </div>
    </div>

  </div>

  <!-- ===================== MAIN GRID ===================== -->
  <div class="grid-main">

    <!-- SENSOR + GPS TELEMETRY -->
    <div class="panel">
      <div class="panel-head">
        <h2><span class="ic">🌡</span> Sensor Telemetry</h2>
        <span class="panel-tag">LIVE</span>
      </div>

      <div class="dataGrid">

        <div class="dataTile">
          <div class="tLabel">Air Temp</div>
          <div class="tValue"><span id="temp">--</span> °C</div>
        </div>

        <div class="dataTile">
          <div class="tLabel">Humidity</div>
          <div class="tValue"><span id="humidity">--</span> %</div>
        </div>

        <div class="dataTile">
          <div class="tLabel">Soil Temp</div>
          <div class="tValue"><span id="soilTemp">--</span> °C</div>
        </div>

        <div class="dataTile">
          <div class="tLabel">Soil Moisture</div>
          <div class="tValue"><span id="moisture">--</span> %</div>
        </div>

        <div class="dataTile">
          <div class="tLabel">pH Level</div>
          <div class="tValue"><span id="ph">--</span></div>
        </div>

        <div class="dataTile">
          <div class="tLabel">Nitrogen (N)</div>
          <div class="tValue"><span id="nitrogen">--</span></div>
        </div>

        <div class="dataTile">
          <div class="tLabel">Phosphorus (P)</div>
          <div class="tValue"><span id="phosphorus">--</span></div>
        </div>

        <div class="dataTile">
          <div class="tLabel">Potassium (K)</div>
          <div class="tValue"><span id="potassium">--</span></div>
        </div>

        <div class="sectionSplit">GPS Position</div>

        <div class="dataTile">
          <div class="tLabel">Latitude</div>
          <div class="tValue"><span id="latitude">--</span></div>
        </div>

        <div class="dataTile">
          <div class="tLabel">Longitude</div>
          <div class="tValue"><span id="longitude">--</span></div>
        </div>

        <div class="dataTile">
          <div class="tLabel">Satellites</div>
          <div class="tValue"><span id="satellites">--</span></div>
        </div>

        <div class="dataTile">
          <div class="tLabel">Speed</div>
          <div class="tValue"><span id="gpsSpeed">--</span> km/h</div>
        </div>

        <div class="dataTile">
          <div class="tLabel">Altitude</div>
          <div class="tValue"><span id="altitude">--</span> m</div>
        </div>

      </div>
    </div>

    <!-- LIVE CAMERA -->
    <div class="panel">
      <div class="panel-head">
        <h2><span class="ic">📷</span> Live Camera Feed</h2>
      </div>

      <div class="cameraContainer">

        <img id="cameraFeed" class="cameraFeed" alt="Live Camera">

        <div class="camHud">
          <div class="camHud-top">
            <div class="liveTag"><span class="dot"></span> LIVE</div>
            <div class="camId">CAMERA 01</div>
          </div>
        </div>

        <div class="camCorners">
          <span class="cc-tl"></span>
          <span class="cc-tr"></span>
          <span class="cc-bl"></span>
          <span class="cc-br"></span>
        </div>

      </div>
    </div>

    <!-- AI ANALYSIS -->
    <div class="panel">
      <div class="panel-head">
        <h2><span class="ic">🧠</span> AI Crop Analysis</h2>
      </div>

      <div class="aiBody">

        <img id="aiImage" class="aiImage" src="" alt="AI Scan">

        <div class="aiDiseaseRow">
          <span id="aiDisease">Waiting...</span>
          <span id="aiSeverity">--</span>
        </div>

        <div class="confBlock">
          <div class="cLabel">
            <span>Confidence</span>
            <span id="aiConfidence">--</span>
          </div>
          <div class="confBar"></div>
        </div>

        <div class="aiRow">
          <b>Recommendation</b>
          <span id="aiRecommendation">--</span>
        </div>

        <div class="placeholder">
          Waiting for AI Analysis...
        </div>

      </div>
    </div>

    <!-- ROVER + PROBE CONTROL -->
    <div class="panel">
      <div class="panel-head">
        <h2><span class="ic">🎮</span> Rover Control</h2>
      </div>

      <div class="controlBody">

        <div class="controlGrid">

          <div></div>
          <button class="forward" onpointerdown="forward();event.preventDefault()">⬆ Fwd</button>
          <div></div>

          <button class="left" onpointerdown="left();event.preventDefault()">⬅ Left</button>
          <button class="stop" onclick="stop()">■ Stop</button>
          <button class="right" onpointerdown="right();event.preventDefault()">➡ Right</button>

          <div></div>
          <button class="backward" onpointerdown="backward();event.preventDefault()">⬇ Back</button>
          <div></div>

        </div>

        <div>
          <div class="subLabel">Probe Control</div>
          <div class="probeGrid">
            <button class="insert" onclick="probeInsert()">Insert</button>
            <button class="retract" onclick="probeRetract()">Retract</button>
            <button class="probeStop" onclick="probeStop()">Stop</button>
          </div>
        </div>

        

      </div>
    </div>

    <!-- GPS MAP -->
    <div class="panel">
      <div class="panel-head">
        <h2><span class="ic">🗺</span> GPS Tracking</h2>

        <div>
          <div class="subLabel">Rover Speed</div>
          <div class="slider">
            <input type="range" min="0" max="255" value="180" oninput="updateSpeed(this.value)">
            <div id="speedValue" class="sliderValue">180</div>
          </div>
        </div>
        
      </div>

      <div class="mapPanel-body">
        <div id="gpsMap"></div>
        <div class="mapOverlay">
          <div class="t1">GPS TRACKING</div>
          <div class="t2">LIVE POSITION</div>
        </div>
      </div>

    </div>

    <!-- MISSION SUMMARY + LOGS -->
    <div class="panel">
      <div class="panel-head">
        <h2><span class="ic">📜</span> Mission &amp; Logs</h2>
      </div>

      <div class="missionStats">

        <div class="missionStat">
          <div class="mLabel">Scanned</div>
          <div class="mValue" id="plantsScanned">0</div>
        </div>

        <div class="missionStat">
          <div class="mLabel">Diseased</div>
          <div class="mValue" id="diseasesDetected">0</div>
        </div>

        <div class="missionStat">
          <div class="mLabel">Healthy</div>
          <div class="mValue" id="healthyPlants">0</div>
        </div>

        <div class="missionStat">
          <div class="mLabel">Mission Time</div>
          <div class="mValue" id="missionTime">00:00</div>
        </div>

      </div>

      <div id="systemLogs" class="systemLogs">System Ready...</div>

    </div>

  </div>

</div>

<script>
%UTILS_JS%
%CONTROL_JS%
%SENSOR_JS%
%STATUS_JS%
%LOG_JS%
%AI_JS%
%DASHBOARD_JS%

(function(){
  var el = document.getElementById('liveClock');
  if(!el) return;
  function tick(){
    var d = new Date();
    var h = String(d.getHours()).padStart(2,'0');
    var m = String(d.getMinutes()).padStart(2,'0');
    var s = String(d.getSeconds()).padStart(2,'0');
    el.textContent = h + ':' + m + ':' + s;
  }
  tick();
  setInterval(tick, 1000);
})();
</script>

<script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>

</body>

</html>

)rawliteral";
