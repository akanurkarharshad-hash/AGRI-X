export type SensorData = {
  temperature?: number; humidity?: number; soilTemperature?: number; moisture?: number; ec?: number; ph?: number;
  nitrogen?: number; phosphorus?: number; potassium?: number; latitude?: number | string; longitude?: number | string;
  satellites?: number; speed?: number; altitude?: number;
};
export type StatusData = Record<string, string>;
export type Snapshot = { online: boolean; checkedAt: string; error?: string; stale?: boolean; sensors: SensorData | null; status: StatusData | null };
export type VisionData = { online: boolean; prediction: Record<string, unknown> | null; checkedAt: string };

async function json<T>(url: string, init?: RequestInit): Promise<T> {
  const response = await fetch(url, { ...init, headers: { 'Content-Type': 'application/json', ...init?.headers } });
  const body = await response.json().catch(() => ({}));
  if (!response.ok) throw new Error((body as { error?: string }).error || `Backend HTTP ${response.status}`);
  return body as T;
}
export const getSnapshot = () => json<Snapshot>('/api/rover/sensors');
export const getStatus = () => json<{ online: boolean; status: StatusData | null; error?: string }>('/api/rover/status');
export const getVision = () => json<VisionData>('/api/vision/latest');
export const control = (action: string) => json<{ online: boolean; action: string; error?: string }>('/api/rover/control', { method: 'POST', body: JSON.stringify({ action }) });
export const setSpeed = (speed: number) => json<{ online: boolean; speed: number; error?: string }>('/api/rover/speed', { method: 'POST', body: JSON.stringify({ speed }) });
