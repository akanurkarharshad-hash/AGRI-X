import { useCallback, useEffect, useMemo, useState } from 'react';
import { Activity, AlertTriangle, ArrowDown, ArrowLeft, ArrowRight, ArrowUp, Battery, Camera, Check, ChevronDown, Circle, Cloud, Compass, Cpu, Crosshair, Droplets, Leaf, MapPin, Menu, Radio, ShieldAlert, Sprout, Thermometer, Wifi, Zap } from 'lucide-react';
import { control, getHistory, getSnapshot, getVision, setSpeed, subscribeTelemetry, type HistoryItem, type Snapshot, type VisionData } from '../api';

const names: Record<string, string> = { forward: 'Forward', backward: 'Reverse', left: 'Turn left', right: 'Turn right', stop: 'Drive stop', probe_insert: 'Arm / probe extend', probe_retract: 'Arm / probe retract', probe_stop: 'Arm / probe stop' };
function stamp(value?: string | null) { return value ? new Date(value).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' }) : '—'; }
function value(v: unknown, suffix = '') { return typeof v === 'number' && Number.isFinite(v) ? `${Number.isInteger(v) ? v : v.toFixed(1)}${suffix}` : '—'; }

export default function App() {
  const [snapshot, setSnapshot] = useState<Snapshot | null>(null);
  const [vision, setVision] = useState<VisionData | null>(null);
  const [history, setHistory] = useState<HistoryItem[]>([]);
  const [online, setOnline] = useState(false);
  const [speed, setSpeedState] = useState(180);
  const [notice, setNotice] = useState('');
  const [logs, setLogs] = useState<string[]>([]);
  const [busy, setBusy] = useState(false);

  const log = useCallback((message: string) => setLogs((items) => [`${new Date().toLocaleTimeString()}  ${message}`, ...items].slice(0, 8)), []);
  const refresh = useCallback(async () => {
    try {
      const data = await getSnapshot();
      setSnapshot(data); setOnline(data.online);
      const stored = await getHistory(60);
      setHistory(stored.items.slice().reverse());
    } catch { setOnline(false); setSnapshot(null); }
    try { setVision(await getVision()); } catch { setVision(null); }
  }, []);
  useEffect(() => {
    void refresh();
    const unsubscribe = subscribeTelemetry((data) => {
      setSnapshot(data); setOnline(data.online);
      if (data.online && !data.stale && data.telemetry) setHistory((items) => {
        if (items.at(-1)?.timestamp === data.telemetry!.timestamp) return items;
        return [...items, data.telemetry!].slice(-60);
      });
    }, () => { setOnline(false); setSnapshot((current) => current ? { ...current, online: false, stale: true } : null); });
    const timer = window.setInterval(() => { void getVision().then(setVision).catch(() => setVision(null)); }, 10000);
    return () => { unsubscribe(); window.clearInterval(timer); };
  }, [refresh]);

  const sensors = snapshot?.sensors;
  const hasGps = online && Boolean(sensors?.gpsFix) && sensors?.latitude !== '--' && typeof sensors?.latitude === 'number' && typeof sensors?.longitude === 'number';
  const currentReading = online && !snapshot?.stale;
  const sensorStatus = sensors ? (currentReading ? 'Current' : 'Stale') : 'Unavailable';
  const send = async (action: string) => {
    setBusy(true); setNotice('');
    try { await control(action); log(names[action] || action); }
    catch (error) { setNotice(error instanceof Error ? error.message : 'Command failed'); log(`Command failed · ${action}`); }
    finally { setBusy(false); }
  };
  const emergency = async () => { await send('stop'); await send('probe_stop'); log('Emergency stop requested'); };
  const onSpeed = async (next: number) => { setSpeedState(next); try { await setSpeed(next); log(`Speed set to ${next}`); } catch (error) { setNotice(error instanceof Error ? error.message : 'Speed update failed'); } };

  const gpsText = hasGps ? `${Number(sensors!.latitude).toFixed(5)}, ${Number(sensors!.longitude).toFixed(5)}` : 'GPS fix unavailable';
  const pred = vision?.prediction;
  const recs = Array.isArray(pred?.recommendations) ? pred.recommendations as string[] : [];
  const quick = useMemo(() => [
    ['Soil moisture', value(sensors?.moisture, '%'), 'field sensor'], ['Soil pH', value(sensors?.ph), 'soil profile'], ['N · P · K', sensors ? `${value(sensors.nitrogen)} · ${value(sensors.phosphorus)} · ${value(sensors.potassium)}` : '—', 'RS485 probe'], ['Air temperature', value(sensors?.temperature, '°C'), 'DHT11'],
  ], [online, sensors]);

  return <div className="app-shell">
    <aside className="rail"><div className="rail-logo"><Sprout size={23}/></div><span className="rail-active"><Activity size={19}/></span><span><Crosshair size={19}/></span><span><Leaf size={19}/></span><span><Radio size={19}/></span><div className="rail-bottom"><span><Menu size={18}/></span></div></aside>
    <main className="workspace">
      <header className="topbar"><div className="brand"><div className="brand-mark"><Sprout size={20}/></div><div><div className="brand-title">AGRI<span>·</span>X</div><div className="brand-sub">FIELD OPERATIONS</div></div><div className="crumb"><span>Workspace</span><ChevronDown size={13}/><b>Rover overview</b></div></div><div className="top-actions"><div className={`link-state ${online ? 'is-online' : 'is-offline'}`}><i/>{online ? 'ROVER CONNECTED' : 'ROVER OFFLINE'}</div><div className="divider"/><div className="top-time"><span>LOCAL SYSTEM TIME</span><b>{new Date().toLocaleTimeString()}</b></div><div className="avatar">AX</div></div></header>
      <div className="page">
        <section className="heading"><div><div className="eyebrow"><span className="eyebrow-line"/> AGRI-X / OPERATIONS</div><h1>Rover overview</h1><p>Field telemetry, vehicle control and crop intelligence in one view.</p></div><div className="heading-right"><div className="mission"><span className="live-dot"/> <div><b>FIELD MONITORING</b><small>SESSION ACTIVE</small></div></div><button className="emergency" onClick={emergency} disabled={!online || busy}><ShieldAlert size={16}/> Emergency stop</button></div></section>
        {notice && <div className="notice"><AlertTriangle size={16}/>{notice}<button onClick={() => setNotice('')}>Dismiss</button></div>}
        <section className="metric-row">{quick.map(([label, metric, detail], i) => <article className="metric-card" key={label}><div className={`metric-icon tone-${i}`}><span>{[<Droplets size={17}/>, <Sprout size={17}/>, <Zap size={17}/>, <Thermometer size={17}/>][i]}</span><small>{currentReading ? 'LIVE' : snapshot?.stale ? 'STALE' : 'NO LINK'}</small></div><div className="metric-label">{label}</div><div className="metric-value">{metric}</div><div className="metric-foot"><span>{detail}</span><span className={currentReading ? 'fresh' : ''}>{currentReading ? '● current' : snapshot?.stale ? `STALE · ${snapshot.age_seconds ?? '—'}s` : '—'}</span></div></article>)}</section>
        <section className="main-grid">
          <article className="panel telemetry"><div className="panel-head"><div><div className="panel-kicker">ROVER TELEMETRY</div><h2>Live field data</h2></div><div className={`pill ${currentReading ? 'green' : 'gray'}`}><i/>{currentReading ? 'Connected · current' : snapshot?.stale ? `Stale · ${snapshot.age_seconds ?? '—'} sec` : 'Offline'}</div></div><div className="sensor-grid">
            <Sensor label="Air temperature" value={value(sensors?.temperature, '°C')} icon={<Thermometer/>} status={sensorStatus}/><Sensor label="Humidity" value={value(sensors?.humidity, '%')} icon={<Droplets/>} status={sensorStatus}/><Sensor label="Soil temperature" value={value(sensors?.soilTemperature, '°C')} icon={<Thermometer/>} status={sensorStatus}/><Sensor label="Soil moisture" value={value(sensors?.moisture, '%')} icon={<Droplets/>} status={sensorStatus}/><Sensor label="Electrical conductivity" value={value(sensors?.ec, ' µS')} icon={<Zap/>} status={sensorStatus}/><Sensor label="pH" value={value(sensors?.ph)} icon={<Activity/>} status={sensorStatus}/><Sensor label="Nitrogen · N" value={value(sensors?.nitrogen)} icon={<Circle/>} status={sensorStatus}/><Sensor label="Phosphorus · P" value={value(sensors?.phosphorus)} icon={<Circle/>} status={sensorStatus}/><Sensor label="Potassium · K" value={value(sensors?.potassium)} icon={<Circle/>} status={sensorStatus}/></div><div className="telemetry-foot"><span><Radio size={13}/> Source: ESP32 sensor gateway</span><span>{currentReading ? `Received ${stamp(snapshot?.checkedAt)}` : snapshot?.stale ? `STALE · Last valid sample ${stamp(snapshot?.checkedAt)}` : 'No current reading'}</span></div></article>
          <article className="panel camera-panel"><div className="panel-head"><div><div className="panel-kicker">FIELD CAMERA</div><h2>Live camera</h2></div><div className="pill gray"><i/>{import.meta.env.VITE_CAMERA_CONFIGURED === 'true' ? 'Configured' : 'Not configured'}</div></div><div className="camera-frame"><img src="/media/camera" onError={(e) => { e.currentTarget.style.display = 'none'; }} alt="Mounted rover camera stream"/><div className="camera-placeholder"><div className="camera-icon"><Camera size={22}/></div><b>Camera feed unavailable</b><span>Set CAMERA_BASE_URL in the laptop environment to connect the mounted phone camera.</span></div><div className="camera-caption"><span><i className="live-dot"/> LIVE VIEW</span><span>ANDROID CAMERA</span></div></div><div className="camera-meta"><span><Wifi size={14}/> Stream routed through laptop backend</span><span>{online ? 'Rover link active' : 'Rover link unavailable'}</span></div></article>
          <article className="panel controls-panel"><div className="panel-head"><div><div className="panel-kicker">VEHICLE OPERATIONS</div><h2>Rover controls</h2></div><span className="subtle"><span className="green-dot"/> {online ? 'Ready' : 'Disconnected'}</span></div><div className="controls-content"><div className="dpad"><span/><button aria-label="Forward" disabled={!online||busy} onClick={() => send('forward')}><ArrowUp/></button><span/><button aria-label="Left" disabled={!online||busy} onClick={() => send('left')}><ArrowLeft/></button><button className="dpad-stop" disabled={!online||busy} onClick={() => send('stop')}><span/><small>STOP</small></button><button aria-label="Right" disabled={!online||busy} onClick={() => send('right')}><ArrowRight/></button><span/><button aria-label="Reverse" disabled={!online||busy} onClick={() => send('backward')}><ArrowDown/></button><span/></div><div className="drive-side"><div className="subsection-title">DRIVE SPEED <b>{speed}<small> / 255</small></b></div><input aria-label="Drive speed" type="range" min="0" max="255" value={speed} onChange={(e) => void onSpeed(Number(e.target.value))}/><div className="range-labels"><span>PRECISION</span><span>MAX OUTPUT</span></div><div className="arm-controls"><div className="subsection-title">ROBOTIC ARM / PROBE <span>MANUAL</span></div><div className="arm-buttons"><button disabled={!online||busy} onClick={() => send('probe_insert')}><ArrowDown size={15}/> Extend</button><button disabled={!online||busy} onClick={() => send('probe_retract')}><ArrowUp size={15}/> Retract</button><button className="arm-stop" disabled={!online||busy} onClick={() => send('probe_stop')}>Stop</button></div></div></div></div><div className="control-note"><ShieldAlert size={14}/> Commands are relayed by the laptop API to the ESP32.</div></article>
          <article className="panel map-panel"><div className="panel-head"><div><div className="panel-kicker">POSITIONING</div><h2>Field map</h2></div><div className={`pill ${hasGps ? 'green' : 'gray'}`}><MapPin size={13}/>{hasGps ? 'GPS FIX' : 'NO FIX'}</div></div><div className="map-canvas"><div className="map-grid-lines"/><div className="map-ring ring-one"/><div className="map-ring ring-two"/><div className="map-crosshair"><Crosshair size={17}/></div>{hasGps && <div className="map-rover" title={gpsText}><Compass size={19}/></div>}<div className="map-label"><MapPin size={14}/>{gpsText}</div><div className="map-zoom"><button>+</button><button>−</button></div><div className="map-attribution">Map tiles not configured</div></div><div className="map-stats"><div><small>LATITUDE</small><b>{hasGps ? Number(sensors!.latitude).toFixed(6) : '—'}</b></div><div><small>LONGITUDE</small><b>{hasGps ? Number(sensors!.longitude).toFixed(6) : '—'}</b></div><div><small>ALTITUDE</small><b>{online ? value(sensors?.altitude, ' m') : '—'}</b></div><div><small>GROUND SPEED</small><b>{online ? value(sensors?.speed, ' km/h') : '—'}</b></div></div></article>
          <article className="panel ai-panel"><div className="panel-head"><div><div className="panel-kicker">COMPUTER VISION</div><h2>YOLO analysis</h2></div><div className={`pill ${vision?.online ? 'green' : 'gray'}`}><i/>{vision?.online ? 'Service online' : 'Unavailable'}</div></div>{pred ? <><div className="prediction"><div className="prediction-image"><img src="/media/latest-image" alt="Latest analysis frame"/></div><div className="prediction-result"><span className="disease-kicker">LATEST OBSERVATION</span><b>{String(pred.disease ?? 'No classification')}</b><span>{String(pred.crop ?? 'Crop unknown')} · {String(pred.severity ?? 'Severity unavailable')}</span></div><div className="confidence"><b>{typeof pred.confidence === 'number' ? `${pred.confidence}%` : '—'}</b><small>MODEL SCORE</small></div></div><div className="rec-list">{recs.length ? recs.map((r) => <div key={r}><Check size={14}/>{r}</div>) : <p>{String(pred.recommendation ?? 'No recommendation provided by the AI service.')}</p>}</div></> : <div className="empty-state"><div className="empty-icon"><Leaf size={20}/></div><b>Waiting for vision service</b><span>Configure AI_BASE_URL to show the current YOLO result. No prediction is inferred while the service is unavailable.</span></div>}<div className="ai-foot"><span><Cpu size={13}/> YOLO inference · laptop service</span><span>{vision?.online ? `Updated ${stamp(vision.checkedAt)}` : 'Not connected'}</span></div></article>
          <article className="panel insight-panel"><div className="panel-head"><div><div className="panel-kicker">FIELD INTELLIGENCE</div><h2>Conditions &amp; insights</h2></div><span className="pill gray">Awaiting data</span></div><div className="insight-list"><Insight icon={<Cloud/>} title="Weather" detail="Weather provider is not configured."/><Insight icon={<AlertTriangle/>} title="Field risk" detail="Risk assessment requires current, validated field and weather data."/><Insight icon={<Activity/>} title="Historical trends" detail={history.length ? `${history.length} accepted samples · moisture and pH history` : 'No validated rover samples have been stored yet.'}/><Insight icon={<Battery/>} title="Mission statistics" detail="Mission duration and distance are not supplied by the current ESP32 API."/></div>{history.length > 1 && <div className="history-spark"><div><span>SOIL MOISTURE</span><b>{value(history.at(-1)?.soil_moisture, '%')}</b></div><svg viewBox="0 0 240 44" preserveAspectRatio="none"><polyline points={sparkline(history.map((item) => item.soil_moisture))}/></svg><div><span>pH</span><b>{value(history.at(-1)?.ph)}</b></div><svg viewBox="0 0 240 44" preserveAspectRatio="none"><polyline className="ph-line" points={sparkline(history.map((item) => item.ph))}/></svg></div>}<div className="insight-note"><ShieldAlert size={15}/> Recommendations and risk indicators remain unavailable until a configured, validated source provides them.</div></article>
          <article className="panel system-panel"><div className="panel-head"><div><div className="panel-kicker">EVENT STREAM</div><h2>System logs</h2></div><button className="clear-button" onClick={() => setLogs([])}>Clear</button></div><div className="system-list">{logs.length ? logs.map((entry, i) => <div className="log-line" key={`${entry}-${i}`}><span className="log-dot"/><code>{entry}</code></div>) : <div className="log-empty"><Activity size={17}/> Events will appear here as telemetry and rover controls update.</div>}</div><div className="system-foot"><span><i className={online ? 'green-dot' : 'red-dot'}/>{online ? 'Gateway polling active' : 'Gateway unavailable'}</span><span>Polling · 2 sec</span></div></article>
        </section>
        <footer className="footer"><span>AGRI-X FIELD OPERATIONS <b>·</b> LAPTOP HOST</span><span><span className={online ? 'green-dot' : 'red-dot'}/>{online ? 'ESP32 gateway connected' : 'ESP32 gateway offline'} <b>·</b> Last check {stamp(snapshot?.checkedAt)}</span></footer>
      </div>
    </main>
  </div>;
}

function Sensor({ label, value: reading, icon, status }: { label: string; value: string; icon: React.ReactNode; status: string }) {
  return <div className="sensor-tile"><div className="sensor-icon">{icon}</div><div className="sensor-copy"><span>{label}</span><b>{reading}</b><small>{status}</small></div></div>;
}
function Insight({ icon, title, detail }: { icon: React.ReactNode; title: string; detail: string }) {
  return <div className="insight-row"><div className="insight-icon">{icon}</div><div><b>{title}</b><span>{detail}</span></div><span className="unavailable">—</span></div>;
}
function sparkline(values: number[]) {
  const low = Math.min(...values); const high = Math.max(...values); const spread = high - low || 1;
  return values.map((item, index) => `${(index / Math.max(values.length - 1, 1)) * 240},${40 - ((item - low) / spread) * 34}`).join(' ');
}
