import 'dotenv/config';
import express, { type Request, type Response } from 'express';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import dotenv from 'dotenv';

const currentDir = path.dirname(fileURLToPath(import.meta.url));
dotenv.config({ path: path.resolve(currentDir, '../../.env') });

const app = express();
app.use(express.json({ limit: '1mb' }));

const port = Number(process.env.BACKEND_PORT || 8000);
const esp32Base = (process.env.ESP32_BASE_URL || '').trim().replace(/\/$/, '');
const cameraBase = (process.env.CAMERA_BASE_URL || '').trim().replace(/\/$/, '');
const aiBase = (process.env.AI_BASE_URL || '').trim().replace(/\/$/, '');
const timeoutMs = 1800;
const attempts = 2;

type JsonMap = Record<string, unknown>;
type ProxyResult = { online: boolean; data?: JsonMap; error?: string; checkedAt: string };
const isoNow = () => new Date().toISOString();

async function requestDevice(route: string): Promise<ProxyResult> {
  if (!esp32Base) return { online: false, error: 'ESP32_BASE_URL is not configured', checkedAt: isoNow() };
  let lastError = 'ESP32 unavailable';
  for (let attempt = 0; attempt < attempts; attempt += 1) {
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), timeoutMs);
    try {
      const response = await fetch(`${esp32Base}${route}`, { signal: controller.signal, cache: 'no-store' });
      if (!response.ok) throw new Error(`ESP32 returned HTTP ${response.status}`);
      const text = await response.text();
      let data: JsonMap = {};
      try { data = JSON.parse(text) as JsonMap; } catch { data = { message: text }; }
      return { online: true, data, checkedAt: isoNow() };
    } catch (error) {
      lastError = error instanceof Error && error.name === 'AbortError' ? 'ESP32 request timed out' : (error instanceof Error ? error.message : 'ESP32 request failed');
      if (attempt + 1 < attempts) await new Promise((resolve) => setTimeout(resolve, 180));
    } finally { clearTimeout(timeout); }
  }
  return { online: false, error: lastError, checkedAt: isoNow() };
}

async function optionalJson(base: string, route: string): Promise<JsonMap | null> {
  if (!base) return null;
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), timeoutMs);
  try {
    const response = await fetch(`${base}${route}`, { signal: controller.signal, cache: 'no-store' });
    if (!response.ok) return null;
    return await response.json() as JsonMap;
  } catch { return null; } finally { clearTimeout(timeout); }
}

app.get('/api/rover/status', async (_req: Request, res: Response) => {
  const result = await requestDevice('/status');
  res.status(result.online ? 200 : 503).json({ ...result, status: result.online ? result.data : null });
});

app.get('/api/rover/sensors', async (_req: Request, res: Response) => {
  const [result, status] = await Promise.all([requestDevice('/sensorData'), requestDevice('/status')]);
  res.status(result.online ? 200 : 503).json({
    online: result.online,
    checkedAt: result.checkedAt,
    error: result.error,
    stale: false,
    sensors: result.online ? result.data : null,
    status: status.online ? status.data : null,
  });
});

const controlRoutes: Record<string, string> = {
  forward: '/forward', backward: '/backward', left: '/left', right: '/right', stop: '/stop',
  probe_insert: '/probe_insert', probe_retract: '/probe_retract', probe_stop: '/probe_stop',
};
app.post('/api/rover/control', async (req: Request, res: Response) => {
  const action = typeof req.body?.action === 'string' ? req.body.action : '';
  const route = controlRoutes[action];
  if (!route) return res.status(400).json({ error: 'Unsupported rover action' });
  const result = await requestDevice(route);
  res.status(result.online ? 200 : 503).json({ ...result, action });
});

app.post('/api/rover/speed', async (req: Request, res: Response) => {
  const speed = Number(req.body?.speed);
  if (!Number.isInteger(speed) || speed < 0 || speed > 255) return res.status(400).json({ error: 'speed must be an integer from 0 to 255' });
  const result = await requestDevice(`/speed?value=${speed}`);
  res.status(result.online ? 200 : 503).json({ ...result, speed });
});

app.get('/api/vision/latest', async (_req, res) => {
  const [latest, running] = await Promise.all([optionalJson(aiBase, '/latest'), optionalJson(aiBase, '/')]);
  res.json({ online: Boolean(latest || running), prediction: latest, service: running, checkedAt: isoNow() });
});

app.get('/media/camera', async (req, res) => {
  if (!cameraBase) return res.status(503).json({ error: 'CAMERA_BASE_URL is not configured' });
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), timeoutMs);
  try {
    const upstream = await fetch(`${cameraBase}/video`, { signal: controller.signal });
    if (!upstream.ok || !upstream.body) return res.status(502).end();
    res.setHeader('Content-Type', upstream.headers.get('content-type') || 'multipart/x-mixed-replace; boundary=frame');
    res.setHeader('Cache-Control', 'no-store');
    const reader = upstream.body.getReader();
    req.on('close', () => { void reader.cancel(); controller.abort(); });
    while (true) {
      const { done, value } = await reader.read();
      if (done) break;
      res.write(value);
    }
    res.end();
  } catch { if (!res.headersSent) res.status(503).json({ error: 'Camera stream unavailable' }); }
  finally { clearTimeout(timeout); }
});

app.get('/media/latest-image', async (_req, res) => {
  if (!aiBase) return res.status(503).end();
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), timeoutMs);
  try {
    const upstream = await fetch(`${aiBase}/static/latest.jpg`, { signal: controller.signal });
    if (!upstream.ok || !upstream.body) return res.status(502).end();
    res.setHeader('Content-Type', upstream.headers.get('content-type') || 'image/jpeg');
    res.setHeader('Cache-Control', 'no-store');
    const reader = upstream.body.getReader();
    while (true) { const { done, value } = await reader.read(); if (done) break; res.write(value); }
    res.end();
  } catch { if (!res.headersSent) res.status(503).end(); } finally { clearTimeout(timeout); }
});

const clientDist = path.resolve(currentDir, '../dist');
app.use(express.static(clientDist));
app.get('*', (_req, res) => res.sendFile(path.join(clientDist, 'index.html')));

app.listen(port, '0.0.0.0', () => console.log(`AGRI-X dashboard backend listening on http://localhost:${port}`));
