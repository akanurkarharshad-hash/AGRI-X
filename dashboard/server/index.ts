import express, { type Request, type Response } from 'express';
import path from 'node:path';
import { config, serverDir } from './config.ts';
import { log } from './logger.ts';
import { telemetryCollector } from './telemetry/collector.ts';
import { readHistory } from './telemetry/store.ts';
import { requestEsp32 } from './telemetry/esp32Client.ts';

const app = express();
app.disable('x-powered-by');
app.use(express.json({ limit: '1mb' }));

const controlRoutes: Record<string, string> = {
  forward: '/forward', backward: '/backward', left: '/left', right: '/right', stop: '/stop',
  probe_insert: '/probe_insert', probe_retract: '/probe_retract', probe_stop: '/probe_stop',
};

function sendMedia(base: string, route: string, contentType: string, res: Response): void {
  if (!base) { res.status(503).json({ error: 'Media service base URL is not configured' }); return; }
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), config.requestTimeoutMs);
  void (async () => {
    try {
      const response = await fetch(`${base}${route}`, { signal: controller.signal, cache: 'no-store' });
      clearTimeout(timeout);
      if (!response.ok || !response.body) { res.status(502).end(); return; }
      res.setHeader('Content-Type', response.headers.get('content-type') || contentType);
      res.setHeader('Cache-Control', 'no-store');
      const reader = response.body.getReader();
      res.on('close', () => { void reader.cancel(); controller.abort(); });
      while (true) {
        const { done, value } = await reader.read();
        if (done) break;
        if (!res.write(value)) await new Promise<void>((resolve) => res.once('drain', resolve));
      }
      res.end();
    } catch (error) {
      if (!res.headersSent) res.status(503).json({ error: error instanceof Error && error.name === 'AbortError' ? 'Media request timed out' : 'Media service unavailable' });
    } finally { clearTimeout(timeout); }
  })();
}

app.get('/health/live', (_req, res) => res.json({ ok: true, service: 'agri-x-backend', timestamp: new Date().toISOString() }));
app.get('/health/ready', (_req, res) => {
  const state = telemetryCollector.latestResponse();
  res.status(telemetryCollector.isStarted ? 200 : 503).json({ ok: telemetryCollector.isStarted, collector_started: telemetryCollector.isStarted, esp32_online: state.online, telemetry_stale: state.stale, checked_at: state.checked_at });
});
app.get('/api/health', (_req, res) => res.json({ ok: true, collector: telemetryCollector.latestResponse() }));

app.get('/api/telemetry/latest', (_req, res) => res.json(telemetryCollector.latestResponse()));

app.get('/api/telemetry/history', async (req, res, next) => {
  try {
    const requested = Number(req.query.limit ?? 100);
    const limit = Number.isInteger(requested) ? Math.max(1, Math.min(requested, config.historyLimit)) : 100;
    const before = typeof req.query.before === 'string' && Number.isFinite(Date.parse(req.query.before)) ? new Date(req.query.before).toISOString() : undefined;
    const items = await readHistory(limit, before);
    res.json({ count: items.length, items });
  } catch (error) { next(error); }
});

app.get('/api/telemetry/events', (req, res) => {
  res.status(200);
  res.setHeader('Content-Type', 'text/event-stream; charset=utf-8');
  res.setHeader('Cache-Control', 'no-cache, no-transform');
  res.setHeader('Connection', 'keep-alive');
  res.flushHeaders();
  const write = (event: { type: string; data: unknown }) => res.write(`event: ${event.type}\ndata: ${JSON.stringify(event.data)}\n\n`);
  write({ type: 'snapshot', data: telemetryCollector.latestResponse() });
  const unsubscribe = telemetryCollector.subscribe(write);
  const heartbeat = setInterval(() => res.write(`: heartbeat ${Date.now()}\n\n`), 15000);
  req.on('close', () => { clearInterval(heartbeat); unsubscribe(); });
});

app.get('/api/rover/status', (_req, res) => res.json(telemetryCollector.statusResponse()));

// Compatibility response for the first laptop UI revision; raw readings still originate at the collector.
app.get('/api/rover/sensors', (_req, res) => {
  const result = telemetryCollector.latestResponse();
  const t = result.telemetry;
  const sensors = t ? {
    temperature: t.temperature, humidity: t.humidity, soilTemperature: t.soil_temperature, moisture: t.soil_moisture,
    ph: t.ph, ec: t.ec, nitrogen: t.nitrogen, phosphorus: t.phosphorus, potassium: t.potassium,
    latitude: t.latitude ?? '--', longitude: t.longitude ?? '--', satellites: t.satellites,
    speed: t.gps_speed, altitude: t.altitude,
  } : null;
  res.json({ online: result.online, stale: result.stale, checkedAt: result.checked_at, error: result.last_error, sensors, status: telemetryCollector.statusResponse().status });
});

app.post('/api/rover/control', async (req: Request, res: Response) => {
  const action = typeof req.body?.action === 'string' ? req.body.action : '';
  const route = controlRoutes[action];
  if (!route) { res.status(400).json({ error: 'Unsupported rover action' }); return; }
  try {
    const result = await requestEsp32(route, false, true);
    log('info', 'rover_command_sent', { action });
    res.json({ online: true, action, response: result });
  } catch (error) {
    const message = error instanceof Error ? error.message : 'Rover command failed';
    log('error', 'rover_command_failed', { action, message });
    res.status(503).json({ online: false, action, error: message });
  }
});

app.post('/api/rover/speed', async (req: Request, res: Response) => {
  const speed = Number(req.body?.speed);
  if (!Number.isInteger(speed) || speed < 0 || speed > 255) { res.status(400).json({ error: 'speed must be an integer from 0 to 255' }); return; }
  try {
    const response = await requestEsp32(`/speed?value=${speed}`, false, true);
    log('info', 'rover_speed_set', { speed });
    res.json({ online: true, speed, response });
  } catch (error) {
    const message = error instanceof Error ? error.message : 'Speed update failed';
    log('error', 'rover_speed_failed', { message });
    res.status(503).json({ online: false, speed, error: message });
  }
});

app.get('/api/vision/latest', async (_req, res) => {
  if (!config.aiBaseUrl) { res.json({ online: false, prediction: null, checkedAt: new Date().toISOString() }); return; }
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), config.requestTimeoutMs);
  try {
    const response = await fetch(`${config.aiBaseUrl}/latest`, { signal: controller.signal, cache: 'no-store' });
    if (!response.ok) { res.json({ online: false, prediction: null, checkedAt: new Date().toISOString() }); return; }
    res.json({ online: true, prediction: await response.json(), checkedAt: new Date().toISOString() });
  } catch { res.json({ online: false, prediction: null, checkedAt: new Date().toISOString() }); }
  finally { clearTimeout(timeout); }
});

app.get('/media/camera', (_req, res) => sendMedia(config.cameraBaseUrl, '/video', 'multipart/x-mixed-replace; boundary=frame', res));
app.get('/media/latest-image', (_req, res) => sendMedia(config.aiBaseUrl, '/static/latest.jpg', 'image/jpeg', res));

app.use((error: unknown, _req: Request, res: Response, _next: express.NextFunction) => {
  const message = error instanceof Error ? error.message : 'Unexpected backend error';
  log('error', 'http_request_failed', { message });
  res.status(500).json({ error: 'Backend request failed' });
});

const clientDist = path.resolve(serverDir, '../dist');
app.use(express.static(clientDist));
app.get('*', (_req, res) => res.sendFile(path.join(clientDist, 'index.html')));

void telemetryCollector.start().catch((error) => log('error', 'collector_start_failed', { message: String(error) }));
const server = app.listen(config.port, '0.0.0.0', () => log('info', 'backend_listening', { port: config.port }));
for (const signal of ['SIGINT', 'SIGTERM'] as const) {
  process.on(signal, () => { telemetryCollector.stop(); server.close(() => process.exit(0)); });
}
