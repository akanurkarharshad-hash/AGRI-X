#pragma once

const char dashboardCSS[] PROGMEM = R"rawliteral(

:root{
  --bg:#0B0F14;
  --bg-grid:#0D1218;
  --panel:#121821;
  --panel-2:#161D27;
  --panel-3:#1A222E;
  --border:rgba(255,255,255,0.08);
  --border-strong:rgba(255,255,255,0.16);
  --primary:#33C97A;
  --primary-dim:rgba(51,201,122,0.14);
  --secondary:#2E8FFF;
  --secondary-dim:rgba(46,143,255,0.14);
  --warning:#F5A524;
  --warning-dim:rgba(245,165,36,0.14);
  --danger:#F0453A;
  --danger-dim:rgba(240,69,58,0.14);
  --text:#EDF2F7;
  --sub:#8996A6;
  --sub-dim:#5C6779;
  --mono:Consolas,'SFMono-Regular',Menlo,monospace;
  --sans:'Segoe UI',system-ui,-apple-system,Arial,sans-serif;
}

*{margin:0;padding:0;box-sizing:border-box;font-family:var(--sans);}

html,body{height:100%;overflow:hidden;}

body{
  background:
    radial-gradient(circle at 15% 0%, rgba(51,201,122,0.06), transparent 40%),
    radial-gradient(circle at 85% 100%, rgba(46,143,255,0.05), transparent 40%),
    var(--bg);
  color:var(--text);
}

::-webkit-scrollbar{width:6px;height:6px;}
::-webkit-scrollbar-track{background:transparent;}
::-webkit-scrollbar-thumb{background:var(--border-strong);border-radius:6px;}

.dashboard{
  height:100vh;
  width:100%;
  display:flex;
  flex-direction:column;
  padding:14px;
  gap:10px;
}

/* ===================== HEADER ===================== */

.topbar{
  height:74px;
  min-height:74px;
  background:linear-gradient(135deg,#121824,#0D1219);
  border:1px solid var(--border);
  border-radius:14px;
  display:flex;
  align-items:center;
  justify-content:space-between;
  padding:0 20px;
  box-shadow:0 4px 18px rgba(0,0,0,.35);
}

.brand{
  display:flex;
  align-items:center;
  gap:12px;
}

.brand-icon{
  width:42px;
  height:42px;
  border-radius:10px;
  background:linear-gradient(135deg,var(--primary),#1E8F53);
  display:flex;
  align-items:center;
  justify-content:center;
  font-size:22px;
  box-shadow:0 0 0 1px var(--border-strong), 0 4px 12px rgba(51,201,122,.25);
}

.brand-text h1{
  font-size:19px;
  letter-spacing:1.5px;
  color:var(--text);
  line-height:1.1;
}

.brand-text span{
  display:block;
  font-size:10.5px;
  letter-spacing:1.5px;
  color:var(--sub);
  margin-top:3px;
}

.statusRow{
  display:flex;
  align-items:center;
  gap:8px;
}

.statusPill{
  display:flex;
  align-items:center;
  gap:8px;
  background:var(--panel-2);
  border:1px solid var(--border);
  border-radius:10px;
  padding:8px 12px;
  min-width:104px;
}

.statusPill-label{
  font-size:9.5px;
  letter-spacing:1px;
  color:var(--sub-dim);
  font-weight:600;
  display:block;
}

.statusValue{
  font-size:12px;
  font-weight:700;
  letter-spacing:.4px;
  display:flex;
  align-items:center;
  gap:6px;
}

.statusValue::before{
  content:"";
  width:7px;
  height:7px;
  border-radius:50%;
  background:currentColor;
  box-shadow:0 0 6px currentColor;
}

.online{color:#2EE68A;}
.offline{color:#F0453A;}

.headerRight{
  display:flex;
  align-items:center;
  gap:16px;
}

.clockBox{
  text-align:right;
  border-left:1px solid var(--border);
  padding-left:16px;
}

.clockBox #liveClock{
  font-family:var(--mono);
  font-size:16px;
  font-weight:700;
  color:var(--secondary);
}

.clockBox .clockLabel{
  font-size:9.5px;
  letter-spacing:1.5px;
  color:var(--sub-dim);
}

/* ===================== MAIN GRID ===================== */

.grid-main{
  flex:1;
  min-height:0;
  display:grid;
  grid-template-columns:300px 1fr 320px;
  grid-template-rows:1fr 1fr;
  gap:10px;
}

.panel{
  background:var(--panel);
  border:1px solid var(--border);
  border-radius:14px;
  padding:14px 16px;
  display:flex;
  flex-direction:column;
  min-height:0;
  box-shadow:0 4px 14px rgba(0,0,0,.25);
  transition:border-color .2s ease;
}

.panel:hover{border-color:var(--border-strong);}

.panel-head{
  display:flex;
  align-items:center;
  justify-content:space-between;
  margin-bottom:10px;
  flex-shrink:0;
}

.panel-head h2{
  font-size:12.5px;
  letter-spacing:1.2px;
  text-transform:uppercase;
  color:var(--sub);
  font-weight:700;
  display:flex;
  align-items:center;
  gap:8px;
}

.panel-head h2 .ic{font-size:15px;}

.panel-tag{
  font-size:9.5px;
  letter-spacing:.8px;
  color:var(--primary);
  background:var(--primary-dim);
  border:1px solid rgba(51,201,122,.3);
  padding:3px 8px;
  border-radius:20px;
  font-weight:700;
}

/* ===================== SENSOR / DATA GRID ===================== */

.dataGrid{
  display:grid;
  grid-template-columns:repeat(2,1fr);
  gap:6px;
  overflow-y:auto;
  padding-right:2px;
}

.dataTile{
  background:var(--panel-2);
  border:1px solid var(--border);
  border-radius:9px;
  padding:7px 9px;
  min-height:44px;
  display:flex;
  flex-direction:column;
  justify-content:center;
}

.dataTile .tLabel{
  font-size:9.5px;
  letter-spacing:.4px;
  color:var(--sub-dim);
  text-transform:uppercase;
  margin-bottom:2px;
}

.dataTile .tValue{
  font-size:14px;
  font-weight:700;
  color:var(--text);
  font-family:var(--mono);
}

.dataGrid .sectionSplit{
  grid-column:1 / -1;
  font-size:9.5px;
  letter-spacing:1.2px;
  color:var(--sub-dim);
  text-transform:uppercase;
  border-top:1px solid var(--border);
  padding-top:6px;
  margin-top:2px;
}

/* legacy .sensor rows kept for compatibility, unused by default layout */
.sensor{display:flex;justify-content:space-between;padding:6px 0;border-bottom:1px solid rgba(255,255,255,.06);font-size:13px;}
.sensor:last-child{border-bottom:none;}
.sensorLabel{color:var(--sub);}
.sensorValue{font-weight:bold;color:var(--text);}

/* ===================== CAMERA ===================== */

.cameraContainer{
  position:relative;
  width:100%;
  height:100%;
  overflow:hidden;
  border-radius:12px;
  background:#000;
  display:flex;
  justify-content:center;
  align-items:center;
  border:1px solid var(--border);
}

.cameraFeed{
  width:100%;
  height:100%;
  object-fit:cover;
  border-radius:12px;
}

.camHud{
  position:absolute;
  inset:0;
  pointer-events:none;
  display:flex;
  flex-direction:column;
  justify-content:space-between;
  padding:12px;
}

.camHud-top{
  display:flex;
  justify-content:space-between;
  align-items:flex-start;
}

.liveTag{
  display:flex;
  align-items:center;
  gap:6px;
  background:rgba(0,0,0,.55);
  border:1px solid rgba(240,69,58,.5);
  color:#FF6259;
  font-size:11px;
  font-weight:700;
  letter-spacing:1px;
  padding:5px 10px;
  border-radius:20px;
  backdrop-filter:blur(4px);
}

.liveTag .dot{
  width:7px;height:7px;border-radius:50%;
  background:#FF6259;
  box-shadow:0 0 8px #FF6259;
  animation:pulse 1.4s infinite;
}

@keyframes pulse{
  0%,100%{opacity:1;}
  50%{opacity:.35;}
}

.camId{
  background:rgba(0,0,0,.55);
  border:1px solid var(--border-strong);
  color:var(--sub);
  font-size:10.5px;
  letter-spacing:1px;
  padding:5px 10px;
  border-radius:20px;
  backdrop-filter:blur(4px);
}

.camCorners span{
  position:absolute;
  width:16px;height:16px;
  border:2px solid rgba(255,255,255,.35);
}
.cc-tl{top:10px;left:10px;border-right:none;border-bottom:none;}
.cc-tr{top:10px;right:10px;border-left:none;border-bottom:none;}
.cc-bl{bottom:10px;left:10px;border-right:none;border-top:none;}
.cc-br{bottom:10px;right:10px;border-left:none;border-top:none;}

/* ===================== AI PANEL ===================== */

.aiBody{
  display:flex;
  flex-direction:column;
  gap:8px;
  overflow-y:auto;
  min-height:0;
}

.aiImage{
  width:100%;
  max-height:96px;
  object-fit:cover;
  border-radius:10px;
  border:1px solid var(--border);
  background:var(--panel-2);
}

.aiDiseaseRow{
  display:flex;
  align-items:center;
  justify-content:space-between;
  gap:8px;
}

#aiDisease{
  font-size:17px;
  font-weight:800;
  color:var(--text);
  letter-spacing:.2px;
}

#aiSeverity{
  font-size:10.5px;
  font-weight:700;
  letter-spacing:.6px;
  text-transform:uppercase;
  padding:4px 10px;
  border-radius:20px;
  background:var(--warning-dim);
  border:1px solid rgba(245,165,36,.35);
  color:var(--warning);
  white-space:nowrap;
}

.confBlock .cLabel{
  display:flex;
  justify-content:space-between;
  font-size:10.5px;
  color:var(--sub-dim);
  letter-spacing:.5px;
  text-transform:uppercase;
  margin-bottom:5px;
}

#aiConfidence{
  font-family:var(--mono);
  font-weight:700;
  color:var(--secondary);
}

.confBar{
  height:7px;
  border-radius:20px;
  background:var(--panel-2);
  border:1px solid var(--border);
  overflow:hidden;
}

.confBar::after{
  content:"";
  display:block;
  height:100%;
  width:65%;
  border-radius:20px;
  background:linear-gradient(90deg,var(--secondary),var(--primary));
}

.aiRow{
  font-size:12.5px;
  color:var(--sub);
  line-height:1.5;
  background:var(--panel-2);
  border:1px solid var(--border);
  border-radius:9px;
  padding:8px 10px;
}

.aiRow b{
  display:block;
  font-size:9.5px;
  letter-spacing:.8px;
  text-transform:uppercase;
  color:var(--sub-dim);
  margin-bottom:3px;
}

#aiRecommendation{color:var(--text);font-weight:600;}

.placeholder{
  flex:1;
  display:flex;
  align-items:center;
  justify-content:center;
  font-size:12px;
  border:1.5px dashed var(--border-strong);
  border-radius:10px;
  color:var(--sub-dim);
  background:var(--panel-2);
  text-align:center;
  padding:10px;
}

/* ===================== ROVER / PROBE CONTROL ===================== */

.controlBody{
  display:flex;
  flex-direction:column;
  gap:10px;
  overflow-y:auto;
  min-height:0;
}

.controlGrid{
  display:grid;
  grid-template-columns:repeat(3,1fr);
  gap:8px;
}

.controlGrid button{
  height:46px;
  font-size:12.5px;
  border:none;
  border-radius:10px;
  cursor:pointer;
  transition:.2s ease;
  font-weight:700;
  color:white;
  letter-spacing:.3px;
}

.controlGrid button:hover{filter:brightness(1.12);transform:translateY(-1px);}
.controlGrid button:active{transform:translateY(0);filter:brightness(.95);}

.forward{background:linear-gradient(135deg,#2ECC71,#219150);}
.backward{background:linear-gradient(135deg,#2E8FFF,#1C64C7);}
.left{background:linear-gradient(135deg,#F5A524,#C97F0E);}
.right{background:linear-gradient(135deg,#F5A524,#C97F0E);}
.stop{background:linear-gradient(135deg,#F0453A,#B92E26);}

.subLabel{
  font-size:9.5px;
  letter-spacing:1px;
  text-transform:uppercase;
  color:var(--sub-dim);
  font-weight:700;
  margin-bottom:6px;
  margin-top:2px;
}

.probeGrid{
  display:grid;
  grid-template-columns:repeat(3,1fr);
  gap:8px;
}

.probeGrid button{
  height:38px;
  border:none;
  border-radius:9px;
  font-size:11.5px;
  cursor:pointer;
  font-weight:700;
  color:white;
  transition:.2s ease;
}

.probeGrid button:hover{filter:brightness(1.12);}

.insert{background:linear-gradient(135deg,#33C97A,#238A54);}
.retract{background:linear-gradient(135deg,#2E8FFF,#1C64C7);}
.probeStop{background:linear-gradient(135deg,#F0453A,#B92E26);}

.slider{width:100%;}

.slider input{
  width:100%;
  accent-color:var(--primary);
}

.sliderValue{
  margin-top:3px;
  font-size:14px;
  text-align:center;
  color:var(--primary);
  font-weight:600;
  font-family:var(--mono);
}

.emergency{
  width:20%;
  height:48px;
  background:linear-gradient(135deg,#F0453A,#B92E26);
  color:white;
  font-size:7px;
  font-weight:400;
  letter-spacing:1px;
  border:1px solid rgba(255,255,255,.15);
  border-radius:11px;
  cursor:pointer;
  transition:.2s ease;
  box-shadow:0 4px 14px rgba(240,69,58,.3);
  flex-shrink:0;
}

.emergency:hover{background:linear-gradient(135deg,#FF564A,#C7362C);transform:translateY(-1px);}
.emergency:active{transform:translateY(0);}

/* ===================== MAP ===================== */

.mapPanel-body{
  position:relative;
  flex:1;
  min-height:0;
  border-radius:12px;
  overflow:hidden;
  border:1px solid var(--border);
}

#gpsMap{
  width:50%;
  height:100%;
  background:#111;
}

.mapOverlay{
  position:absolute;
  top:10px;
  left:10px;
  z-index:1000;
  background:rgba(10,14,20,.75);
  border:1px solid var(--border-strong);
  border-radius:9px;
  padding:6px 10px;
  backdrop-filter:blur(4px);
  pointer-events:none;
}

.mapOverlay .t1{
  font-size:10px;
  font-weight:700;
  letter-spacing:1px;
  color:var(--text);
}

.mapOverlay .t2{
  font-size:9px;
  letter-spacing:.8px;
  color:var(--primary);
  display:flex;
  align-items:center;
  gap:4px;
  margin-top:2px;
}

.mapOverlay .t2::before{
  content:"";
  width:6px;height:6px;border-radius:50%;
  background:var(--primary);
  box-shadow:0 0 6px var(--primary);
}

/* ===================== MISSION / LOGS ===================== */

.missionStats{
  display:grid;
  grid-template-columns:repeat(4,1fr);
  gap:6px;
  margin-bottom:10px;
  flex-shrink:0;
}

.missionStat{
  background:var(--panel-2);
  border:1px solid var(--border);
  border-radius:9px;
  padding:8px 4px;
  text-align:center;
}

.missionStat .mLabel{
  font-size:8px;
  letter-spacing:.3px;
  color:var(--sub-dim);
  text-transform:uppercase;
  line-height:1.2;
  min-height:18px;
  display:flex;
  align-items:center;
  justify-content:center;
}

.missionStat .mValue{
  font-size:16px;
  font-weight:800;
  color:var(--primary);
  font-family:var(--mono);
  margin-top:2px;
}

.systemLogs{
  flex:1;
  min-height:0;
  overflow-y:auto;
  background:#0D1218;
  border-radius:10px;
  padding:10px 12px;
  font-size:11.5px;
  line-height:1.6;
  font-family:var(--mono);
  border:1px solid var(--border);
  color:var(--sub);
}

/* ===================== RESPONSIVE ===================== */

@media(max-width:1500px){
  .grid-main{grid-template-columns:270px 1fr 290px;}
}

@media(max-width:1400px){
  .brand-text h1{font-size:16px;}
  .statusPill{min-width:88px;padding:6px 9px;}
}

@media(max-width:1300px){
  html,body{overflow:auto;}
  .dashboard{height:auto;min-height:100vh;}
  .grid-main{
    grid-template-columns:1fr 1fr;
    grid-template-rows:auto auto auto;
  }
  .topbar{flex-wrap:wrap;height:auto;min-height:74px;padding:10px 16px;gap:10px;}
  .statusRow{flex-wrap:wrap;}
}

@media(max-width:800px){
  .grid-main{grid-template-columns:1fr;}
  .controlGrid,.probeGrid{grid-template-columns:repeat(3,1fr);}
  .dataGrid{grid-template-columns:1fr 1fr;}
}

)rawliteral";