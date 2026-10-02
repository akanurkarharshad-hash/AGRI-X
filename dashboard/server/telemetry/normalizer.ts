import { config } from '../config.ts';
import { ValidationError, type CanonicalTelemetry } from './types.ts';

type JsonMap = Record<string, unknown>;

function requiredNumber(source: JsonMap, key: string, min: number, max: number): number {
  const value = source[key];
  if (typeof value !== 'number' || !Number.isFinite(value)) throw new ValidationError(`${key} must be a finite number`);
  if (value < min || value > max) throw new ValidationError(`${key} is outside the accepted range ${min}..${max}`);
  return value;
}

function optionalCoordinate(value: unknown, key: string, min: number, max: number): number | null {
  if (value === '--' || value === null || value === undefined) return null;
  if (typeof value !== 'number' || !Number.isFinite(value)) throw new ValidationError(`${key} must be a finite number or the ESP32 no-fix marker`);
  if (value < min || value > max) throw new ValidationError(`${key} is outside the accepted range ${min}..${max}`);
  return value;
}

export function normalizeEsp32Telemetry(raw: unknown, rawStatus: unknown, timestamp = new Date().toISOString()): CanonicalTelemetry {
  if (!raw || typeof raw !== 'object' || Array.isArray(raw)) throw new ValidationError('sensorData response must be a JSON object');
  const source = raw as JsonMap;
  const status = rawStatus && typeof rawStatus === 'object' && !Array.isArray(rawStatus) ? rawStatus as JsonMap : {};
  const latitude = optionalCoordinate(source.latitude, 'latitude', -90, 90);
  const longitude = optionalCoordinate(source.longitude, 'longitude', -180, 180);
  if ((latitude === null) !== (longitude === null)) throw new ValidationError('latitude and longitude must either both be present or both be unavailable');
  const gpsFix = latitude !== null && longitude !== null && String(status.gps || '').toUpperCase() === 'CONNECTED';
  const normalizedTime = new Date(timestamp);
  if (!Number.isFinite(normalizedTime.getTime())) throw new ValidationError('timestamp must be a valid date');

  return {
    device_id: config.deviceId,
    timestamp: normalizedTime.toISOString(),
    temperature: requiredNumber(source, 'temperature', -50, 85),
    humidity: requiredNumber(source, 'humidity', 0, 100),
    soil_temperature: requiredNumber(source, 'soilTemperature', -40, 80),
    soil_moisture: requiredNumber(source, 'moisture', 0, 100),
    ph: requiredNumber(source, 'ph', 0, 14),
    ec: requiredNumber(source, 'ec', 0, 20000),
    nitrogen: requiredNumber(source, 'nitrogen', 0, 65535),
    phosphorus: requiredNumber(source, 'phosphorus', 0, 65535),
    potassium: requiredNumber(source, 'potassium', 0, 65535),
    latitude,
    longitude,
    gps_speed: requiredNumber(source, 'speed', 0, 500),
    altitude: requiredNumber(source, 'altitude', -12000, 100000),
    satellites: requiredNumber(source, 'satellites', 0, 100),
    gps_fix: gpsFix,
  };
}
