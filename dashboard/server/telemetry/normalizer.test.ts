import assert from 'node:assert/strict';
import test from 'node:test';
import { normalizeEsp32Telemetry } from './normalizer.ts';
import { ValidationError } from './types.ts';

const source = () => ({
  temperature: 22.5, humidity: 61, soilTemperature: 18.2, moisture: 34.1, ph: 6.7, ec: 870,
  nitrogen: 18, phosphorus: 21, potassium: 33, latitude: 12.345, longitude: 76.543,
  satellites: 8, speed: 1.2, altitude: 902,
});

test('normalizes the ESP32 field names and stamps an ISO UTC timestamp', () => {
  const result = normalizeEsp32Telemetry(source(), { gps: 'CONNECTED' }, '2026-10-01T12:00:00Z');
  assert.equal(result.soil_temperature, 18.2);
  assert.equal(result.soil_moisture, 34.1);
  assert.equal(result.gps_speed, 1.2);
  assert.equal(result.gps_fix, true);
  assert.equal(result.timestamp, '2026-10-01T12:00:00.000Z');
});

test('represents unavailable GPS coordinates as null and marks gps_fix false', () => {
  const raw = { ...source(), latitude: '--', longitude: '--' };
  const result = normalizeEsp32Telemetry(raw, { gps: 'SEARCHING' });
  assert.equal(result.latitude, null);
  assert.equal(result.longitude, null);
  assert.equal(result.gps_fix, false);
});

test('rejects a malformed sensor object instead of fabricating values', () => {
  assert.throws(() => normalizeEsp32Telemetry('{not-json}', {}), ValidationError);
  assert.throws(() => normalizeEsp32Telemetry(null, {}), /JSON object/);
});

test('rejects out-of-range and non-finite sensor readings', () => {
  assert.throws(() => normalizeEsp32Telemetry({ ...source(), moisture: 101 }, {}), /moisture is outside/);
  assert.throws(() => normalizeEsp32Telemetry({ ...source(), ph: Number.NaN }, {}), /ph must be a finite number/);
  assert.throws(() => normalizeEsp32Telemetry({ ...source(), humidity: undefined }, {}), /humidity must be a finite number/);
});

test('rejects mismatched GPS coordinates', () => {
  assert.throws(() => normalizeEsp32Telemetry({ ...source(), longitude: '--' }, {}), /both be present/);
});
