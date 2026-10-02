import { config } from '../config.ts';
import { log } from '../logger.ts';
import { requestEsp32 } from './esp32Client.ts';
import { normalizeEsp32Telemetry } from './normalizer.ts';
import { appendTelemetry, loadLatest } from './store.ts';
import { ValidationError, type CanonicalTelemetry, type CollectorHealth, type TelemetryEvent } from './types.ts';

type Listener = (event: TelemetryEvent) => void;
const listeners = new Set<Listener>();
let lastFailureLogAt = 0;

export class TelemetryCollector {
  private timer: NodeJS.Timeout | undefined;
  private running = false;
  private latest: CanonicalTelemetry | null = null;
  private health: CollectorHealth = { online: false, stale: true, age_seconds: null, checked_at: null, last_success_at: null, last_error: null };

  get isStarted(): boolean { return this.running; }

  async start(): Promise<void> {
    this.latest = await loadLatest();
    this.refreshAge();
    this.running = true;
    void this.poll();
    this.timer = setInterval(() => void this.poll(), config.pollIntervalMs);
    this.timer.unref();
    log('info', 'collector_started', { interval_ms: config.pollIntervalMs, stale_after_ms: config.staleAfterMs, has_persisted_sample: Boolean(this.latest) });
  }

  stop(): void { this.running = false; if (this.timer) clearInterval(this.timer); }

  subscribe(listener: Listener): () => void { listeners.add(listener); return () => listeners.delete(listener); }

  latestResponse() {
    this.refreshAge();
    return { ...this.health, telemetry: this.latest };
  }

  statusResponse() {
    this.refreshAge();
    return { ...this.health, status: this.health.online ? this.lastDeviceStatus : null };
  }

  private lastDeviceStatus: Record<string, unknown> | null = null;

  private refreshAge(): void {
    const timestamp = this.latest?.timestamp;
    const ageMs = timestamp ? Math.max(0, Date.now() - Date.parse(timestamp)) : null;
    this.health = {
      ...this.health,
      stale: ageMs === null || ageMs > config.staleAfterMs || !this.health.online,
      age_seconds: ageMs === null ? null : Math.floor(ageMs / 1000),
      last_success_at: timestamp || null,
    };
  }

  private publish(type: TelemetryEvent['type'], data: unknown): void { for (const listener of listeners) listener({ type, data }); }

  private async poll(): Promise<void> {
    if (!this.running) return;
    const checkedAt = new Date().toISOString();
    try {
      const [rawSensors, rawStatus] = await Promise.all([requestEsp32('/sensorData'), requestEsp32('/status')]);
      if (!rawStatus || typeof rawStatus !== 'object' || Array.isArray(rawStatus)) throw new ValidationError('status response must be a JSON object');
      const sample = normalizeEsp32Telemetry(rawSensors, rawStatus, checkedAt);
      await appendTelemetry(sample);
      this.latest = sample;
      this.lastDeviceStatus = rawStatus as Record<string, unknown>;
      this.health = { online: true, stale: false, age_seconds: 0, checked_at: checkedAt, last_success_at: sample.timestamp, last_error: null };
      this.publish('telemetry', this.latestResponse());
      this.publish('status', this.statusResponse());
      log('info', 'telemetry_collected', { device_id: sample.device_id, gps_fix: sample.gps_fix });
    } catch (error) {
      const message = error instanceof Error ? error.message : 'Telemetry collection failed';
      const wasOnline = this.health.online;
      this.health = { ...this.health, online: false, checked_at: checkedAt, last_error: message };
      this.refreshAge();
      this.publish('telemetry', this.latestResponse());
      const now = Date.now();
      if (wasOnline || error instanceof ValidationError || now - lastFailureLogAt >= 15000) {
        log(error instanceof ValidationError ? 'warn' : 'error', error instanceof ValidationError ? 'telemetry_rejected' : 'telemetry_poll_failed', { message });
        lastFailureLogAt = now;
      }
    }
  }
}

export const telemetryCollector = new TelemetryCollector();
