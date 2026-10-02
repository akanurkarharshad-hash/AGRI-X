# AGRI-X laptop dashboard and data gateway

The laptop hosts the browser dashboard and the Node/TypeScript API gateway. The gateway polls the ESP32, validates and normalizes each complete reading, keeps the newest valid reading in memory, writes accepted history to JSON Lines, and streams updates to browsers over Server-Sent Events. Browser JavaScript calls laptop routes only.

The existing `Mini_project_AI` Python analysis modules remain available through their existing Flask application. The ESP32 has no plot/crop association in its telemetry contract, and its old sensor-ingest route writes placeholder zero N/P/K values. The gateway therefore does not forward raw telemetry into that agronomic analysis path until a real plot association and validated data semantics are defined.

## Configure and run

1. Install Node.js 20 or later and npm.
2. Copy the repository root `.env.example` to `.env` and set `ESP32_BASE_URL` to the rover's reachable base URL. The local `.env` is ignored by Git.
3. Set `TELEMETRY_POLL_INTERVAL_MS` and `TELEMETRY_STALE_AFTER_MS` if the field network needs different timing. Defaults are 2 seconds and 10 seconds.
4. Optionally configure `CAMERA_BASE_URL` and `AI_BASE_URL`. Camera and YOLO routes remain proxied through the laptop.
5. From `dashboard/`, run `npm install`, then `npm run dev`.
6. Open the Vite URL printed in the terminal. Vite forwards `/api/*` and `/media/*` to the laptop gateway at `BACKEND_PORT` (default 8000).

For a production build, run `npm run build`, then `npm start` from `dashboard/`. The backend serves the built `dist/` assets on `BACKEND_PORT`.

## Gateway API

- `GET /api/telemetry/latest` returns the latest valid canonical sample and connectivity/freshness metadata. A previous sample is retained after failure but is explicitly marked `stale`; no sample is returned when none has ever validated.
- `GET /api/telemetry/history?limit=100&before=<ISO timestamp>` returns stored valid readings, newest first.
- `GET /api/telemetry/events` streams `snapshot`, `telemetry`, and `status` SSE events, with heartbeat comments.
- `GET /api/rover/status` returns normalized link/status data and collector freshness.
- `GET /api/rover/sensors` remains as a compatibility response for the first laptop UI revision.
- `POST /api/rover/control` with `{ "action": "forward|backward|left|right|stop|probe_insert|probe_retract|probe_stop" }` proxies existing ESP32 routes.
- `POST /api/rover/speed` with `{ "speed": 0..255 }` proxies `/speed?value=...`.
- `GET /health/live`, `GET /health/ready`, and `GET /api/health` provide service health.
- `GET /api/vision/latest`, `/media/camera`, and `/media/latest-image` proxy optional vision/camera sources.

ESP32 requests have a configurable timeout and two attempts for polling. Commands use one attempt to avoid repeating an action. HTTP control responses may be plain text; telemetry and status responses must be valid JSON.

## Canonical telemetry

Every accepted record contains `device_id`, a UTC ISO timestamp, temperature, humidity, soil temperature/moisture, pH, EC, N/P/K, GPS coordinates, GPS speed, altitude, satellites and `gps_fix`. The collector accepts the existing ESP32 JSON field names (`soilTemperature`, `moisture`, and `speed`) and stores the canonical snake_case form. When GPS coordinates are the firmware's `"--"` no-fix marker, latitude/longitude become `null` and `gps_fix` is false; speed/altitude/satellites are only meaningful as a position when `gps_fix` is true.

Readings with missing, nonnumeric, non-finite or out-of-range required fields are rejected as a whole. They do not overwrite latest state or enter history. When the ESP32 is unavailable or a new reading is rejected, API clients receive the prior valid record only with `stale: true`, age and an error reason.

## Persistence and validation

Accepted history defaults to `dashboard/data/telemetry.jsonl` and is ignored by Git. Set `TELEMETRY_HISTORY_FILE` to choose another local path. Structured JSON logs are written to stdout. Backend checks run with `npm run check:backend`; normalization and gateway integration tests run with `npm run test:backend` on Node 22.6 or later. The integration test uses a controllable local ESP32 protocol fixture; a separate live check still requires the physical rover to be reachable at `ESP32_BASE_URL`.
