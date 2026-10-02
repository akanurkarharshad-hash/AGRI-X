export type CanonicalTelemetry = {
  device_id: string;
  timestamp: string;
  temperature: number;
  humidity: number;
  soil_temperature: number;
  soil_moisture: number;
  ph: number;
  ec: number;
  nitrogen: number;
  phosphorus: number;
  potassium: number;
  latitude: number | null;
  longitude: number | null;
  gps_speed: number;
  altitude: number;
  satellites: number;
  gps_fix: boolean;
};

export type CollectorHealth = {
  online: boolean;
  stale: boolean;
  age_seconds: number | null;
  checked_at: string | null;
  last_success_at: string | null;
  last_error: string | null;
};

export type TelemetryEvent = { type: 'snapshot' | 'telemetry' | 'status'; data: unknown };

export class ValidationError extends Error {
  constructor(message: string) { super(message); this.name = 'ValidationError'; }
}
