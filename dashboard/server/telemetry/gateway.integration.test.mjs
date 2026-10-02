import assert from 'node:assert/strict';
import { spawn } from 'node:child_process';
import { once } from 'node:events';
import { createServer } from 'node:http';
import { mkdtemp, rm } from 'node:fs/promises';
import net from 'node:net';
import os from 'node:os';
import path from 'node:path';
import test from 'node:test';
import { fileURLToPath } from 'node:url';

const dashboardDir = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const validReading = {
  temperature: 23.1, humidity: 62, soilTemperature: 19.4, moisture: 31.5, ph: 6.8, ec: 945,
  nitrogen: 22, phosphorus: 18, potassium: 40, latitude: 12.34, longitude: 76.54,
  satellites: 9, speed: 0.4, altitude: 875,
};

async function listen(server, host = '127.0.0.1') {
  server.listen(0, host);
  await once(server, 'listening');
  return server.address().port;
}
async function unusedPort() {
  const server = net.createServer();
  const port = await listen(server);
  await new Promise((resolve, reject) => server.close((error) => error ? reject(error) : resolve()));
  return port;
}
async function waitUntil(fn, timeout = 7000) {
  const end = Date.now() + timeout;
  let last;
  while (Date.now() < end) {
    try { last = await fn(); if (last) return last; } catch {}
    await new Promise((resolve) => setTimeout(resolve, 50));
  }
  throw new Error(`Timed out waiting for gateway state: ${JSON.stringify(last)}`);
}

test('gateway polls, validates, persists, streams, and marks stale telemetry correctly', async (t) => {
  let mode = 'valid';
  const device = createServer((req, res) => {
    if (mode === 'offline') { res.writeHead(503).end('offline'); return; }
    if (req.url === '/sensorData') {
      if (mode === 'malformed') { res.writeHead(200, { 'content-type': 'application/json' }).end('{broken'); return; }
      const reading = mode === 'invalid' ? { ...validReading, moisture: 140 } : mode === 'gps' ? { ...validReading, latitude: '--', longitude: '--', satellites: 0, speed: 0 } : validReading;
      res.writeHead(200, { 'content-type': 'application/json' }).end(JSON.stringify(reading)); return;
    }
    if (req.url === '/status') {
      const gps = mode === 'gps' ? 'SEARCHING' : 'CONNECTED';
      res.writeHead(200, { 'content-type': 'application/json' }).end(JSON.stringify({ wifi: 'ONLINE', gps, soil: 'CONNECTED', dht: 'CONNECTED', rover: 'READY' })); return;
    }
    if (['/forward', '/backward', '/left', '/right', '/stop', '/probe_insert', '/probe_retract', '/probe_stop'].includes(req.url)) {
      res.writeHead(200, { 'content-type': 'text/plain' }).end(req.url.slice(1).toUpperCase()); return;
    }
    if (req.url.startsWith('/speed?value=')) { res.writeHead(200, { 'content-type': 'text/plain' }).end('OK'); return; }
    res.writeHead(404).end();
  });
  const devicePort = await listen(device);
  const backendPort = await unusedPort();
  const tempDir = await mkdtemp(path.join(os.tmpdir(), 'agrix-gateway-'));
  const child = spawn(process.execPath, ['--experimental-strip-types', 'server/index.ts'], {
    cwd: dashboardDir,
    env: {
      ...process.env,
      ESP32_BASE_URL: `http://127.0.0.1:${devicePort}`,
      BACKEND_PORT: String(backendPort),
      TELEMETRY_POLL_INTERVAL_MS: '100',
      TELEMETRY_STALE_AFTER_MS: '3000',
      ESP32_TIMEOUT_MS: '400',
      ESP32_RETRY_DELAY_MS: '30',
      TELEMETRY_HISTORY_FILE: path.join(tempDir, 'telemetry.jsonl'),
    },
    stdio: ['ignore', 'pipe', 'pipe'],
  });
  let childOutput = '';
  child.stdout.on('data', (chunk) => { childOutput += chunk.toString(); });
  child.stderr.on('data', (chunk) => { childOutput += chunk.toString(); });
  t.after(async () => {
    child.kill('SIGTERM');
    await Promise.race([once(child, 'exit'), new Promise((resolve) => setTimeout(resolve, 1500))]);
    await new Promise((resolve) => device.close(() => resolve()));
    await rm(tempDir, { recursive: true, force: true });
  });

  const base = `http://127.0.0.1:${backendPort}`;
  await waitUntil(async () => child.exitCode !== null ? (() => { throw new Error(childOutput); })() : (await fetch(`${base}/health/live`).then((r) => r.ok)));
  const first = await waitUntil(async () => {
    const state = await fetch(`${base}/api/telemetry/latest`).then((r) => r.json());
    return state.online && state.telemetry ? state : null;
  });
  assert.equal(first.telemetry.soil_temperature, validReading.soilTemperature);
  assert.equal(first.telemetry.gps_fix, true);
  assert.match(first.telemetry.timestamp, /Z$/);

  const events = await fetch(`${base}/api/telemetry/events`);
  assert.match(events.headers.get('content-type'), /text\/event-stream/);
  const eventReader = events.body.getReader();
  const initialEvent = await eventReader.read();
  assert.match(new TextDecoder().decode(initialEvent.value), /event: snapshot/);
  await eventReader.cancel();

  const history = await fetch(`${base}/api/telemetry/history?limit=5`).then((r) => r.json());
  assert.ok(history.count >= 1);
  const control = await fetch(`${base}/api/rover/control`, { method: 'POST', headers: { 'content-type': 'application/json' }, body: JSON.stringify({ action: 'forward' }) }).then((r) => r.json());
  assert.equal(control.online, true, 'plain-text ESP32 control response should be accepted');
  const speed = await fetch(`${base}/api/rover/speed`, { method: 'POST', headers: { 'content-type': 'application/json' }, body: JSON.stringify({ speed: 123 }) }).then((r) => r.json());
  assert.equal(speed.online, true);
  assert.equal(speed.speed, 123);
  const invalidSpeed = await fetch(`${base}/api/rover/speed`, { method: 'POST', headers: { 'content-type': 'application/json' }, body: JSON.stringify({ speed: 300 }) });
  assert.equal(invalidSpeed.status, 400);

  mode = 'invalid';
  const rejected = await waitUntil(async () => {
    const state = await fetch(`${base}/api/telemetry/latest`).then((r) => r.json());
    return !state.online && /moisture is outside/.test(state.last_error || '') ? state : null;
  });
  assert.equal(rejected.stale, true);
  assert.equal(rejected.telemetry.soil_moisture, validReading.moisture, 'invalid sensor values must not replace the last valid sample');
  const acceptedBeforeInvalidPolls = rejected.telemetry.timestamp;
  await new Promise((resolve) => setTimeout(resolve, 250));
  const afterInvalidPolls = await fetch(`${base}/api/telemetry/latest`).then((r) => r.json());
  assert.equal(afterInvalidPolls.telemetry.timestamp, acceptedBeforeInvalidPolls, 'repeated invalid samples must not advance the stored sample');

  mode = 'gps';
  const noFix = await waitUntil(async () => {
    const state = await fetch(`${base}/api/telemetry/latest`).then((r) => r.json());
    return state.online && state.telemetry?.gps_fix === false ? state : null;
  });
  assert.equal(noFix.telemetry.latitude, null);
  assert.equal(noFix.telemetry.longitude, null);

  mode = 'malformed';
  const malformed = await waitUntil(async () => {
    const state = await fetch(`${base}/api/telemetry/latest`).then((r) => r.json());
    return !state.online && /malformed JSON/.test(state.last_error || '') ? state : null;
  });
  assert.equal(malformed.stale, true);
  const acceptedBeforeMalformedPolls = malformed.telemetry.timestamp;
  await new Promise((resolve) => setTimeout(resolve, 250));
  const afterMalformedPolls = await fetch(`${base}/api/telemetry/latest`).then((r) => r.json());
  assert.equal(afterMalformedPolls.telemetry.timestamp, acceptedBeforeMalformedPolls);

  mode = 'offline';
  const offline = await waitUntil(async () => {
    const state = await fetch(`${base}/api/rover/status`).then((r) => r.json());
    return !state.online && /HTTP 503/.test(state.last_error || '') ? state : null;
  });
  assert.equal(offline.status, null);
  assert.ok(childOutput.includes('telemetry_collected'));
});
