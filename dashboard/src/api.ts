export type CanonicalTelemetry = {
  device_id: string; timestamp: string; temperature: number; humidity: number; soil_temperature: number; soil_moisture: number;
  ph: number; ec: number; nitrogen: number; phosphorus: number; potassium: number; latitude: number | null; longitude: number | null;
  gps_speed: number; altitude: number; satellites: number; gps_fix: boolean;
};
export type SensorData = {
  temperature: number; humidity: number; soilTemperature: number; moisture: number; ec: number; ph: number;
  nitrogen: number; phosphorus: number; potassium: number; latitude: number | string; longitude: number | string;
  satellites: number; speed: number; altitude: number; gpsFix: boolean;
};
export type Snapshot = {
  online: boolean; stale: boolean; age_seconds: number | null; checkedAt?: string | null; error?: string | null;
  sensors: SensorData | null; status: Record<string, unknown> | null; telemetry: CanonicalTelemetry | null;
};
export type HistoryItem = CanonicalTelemetry;
export type VisionData = { online: boolean; prediction: Record<string, unknown> | null; checkedAt: string };

async function json<T>(url: string, init?: RequestInit): Promise<T> {
  const response = await fetch(url, { ...init, headers: { 'Content-Type': 'application/json', ...init?.headers } });
  const body = await response.json().catch(() => ({}));
  if (!response.ok) throw new Error((body as { error?: string }).error || `Backend HTTP ${response.status}`);
  return body as T;
}

type LatestResponse = {
  online: boolean; stale: boolean; age_seconds: number | null; checked_at: string | null; last_error: string | null;
  telemetry: CanonicalTelemetry | null;
};

function fromLatest(data: LatestResponse): Snapshot {
  const t = data.telemetry;
  return {
    online: data.online,
    stale: data.stale,
    age_seconds: data.age_seconds,
    checkedAt: data.checked_at,
    error: data.last_error,
    sensors: t ? {
      temperature: t.temperature, humidity: t.humidity, soilTemperature: t.soil_temperature, moisture: t.soil_moisture,
      ec: t.ec, ph: t.ph, nitrogen: t.nitrogen, phosphorus: t.phosphorus, potassium: t.potassium,
      latitude: t.latitude ?? '--', longitude: t.longitude ?? '--', gpsFix: t.gps_fix,
      satellites: t.satellites, speed: t.gps_speed, altitude: t.altitude,
    } : null,
    status: null,
    telemetry: t,
  };
}

export async function getSnapshot(): Promise<Snapshot> {
  return fromLatest(await json<LatestResponse>('/api/telemetry/latest'));
}
export const getHistory = async (limit = 60) => json<{ count: number; items: HistoryItem[] }>(`/api/telemetry/history?limit=${limit}`);
export const getVision = () => json<VisionData>('/api/vision/latest');
export const subscribeTelemetry = (onSnapshot: (data: Snapshot) => void, onError?: () => void) => {
  const events = new EventSource('/api/telemetry/events');
  const consume = (event: MessageEvent<string>) => {
    try { onSnapshot(fromLatest(JSON.parse(event.data) as LatestResponse)); } catch { onError?.(); }
  };
  events.addEventListener('snapshot', consume as EventListener);
  events.addEventListener('telemetry', consume as EventListener);
  events.onerror = () => onError?.();
  return () => events.close();
};
export const control = (action: string) => json<{ online: boolean; action: string; error?: string }>('/api/rover/control', { method: 'POST', body: JSON.stringify({ action }) });
export const setSpeed = (speed: number) => json<{ online: boolean; speed: number; error?: string }>('/api/rover/speed', { method: 'POST', body: JSON.stringify({ speed }) });
