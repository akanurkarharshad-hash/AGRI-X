import path from 'node:path';
import { fileURLToPath } from 'node:url';
import dotenv from 'dotenv';

export const serverDir = path.dirname(fileURLToPath(import.meta.url));
dotenv.config({ path: path.resolve(serverDir, '../../.env') });

function positiveNumber(value: string | undefined, fallback: number): number {
  const parsed = Number(value);
  return Number.isFinite(parsed) && parsed > 0 ? parsed : fallback;
}

export const config = {
  port: positiveNumber(process.env.BACKEND_PORT, 8000),
  esp32BaseUrl: (process.env.ESP32_BASE_URL || '').trim().replace(/\/$/, ''),
  cameraBaseUrl: (process.env.CAMERA_BASE_URL || '').trim().replace(/\/$/, ''),
  aiBaseUrl: (process.env.AI_BASE_URL || '').trim().replace(/\/$/, ''),
  deviceId: (process.env.DEVICE_ID || 'AGRI-X-ESP32').trim(),
  pollIntervalMs: positiveNumber(process.env.TELEMETRY_POLL_INTERVAL_MS, 2000),
  staleAfterMs: positiveNumber(process.env.TELEMETRY_STALE_AFTER_MS, 10000),
  requestTimeoutMs: positiveNumber(process.env.ESP32_TIMEOUT_MS, 1800),
  retryDelayMs: positiveNumber(process.env.ESP32_RETRY_DELAY_MS, 200),
  historyLimit: Math.min(10000, positiveNumber(process.env.TELEMETRY_HISTORY_LIMIT, 1000)),
  historyFile: path.resolve(process.env.TELEMETRY_HISTORY_FILE || path.resolve(serverDir, '../data/telemetry.jsonl')),
};
