# AGRI-X laptop dashboard

The laptop hosts both the browser dashboard and the API gateway. Browser code uses same-origin `/api/*` and `/media/*` routes only; the backend reads `ESP32_BASE_URL`, `CAMERA_BASE_URL`, and `AI_BASE_URL` from the repository root `.env`.

## Run locally

1. Install Node.js 20 or later.
2. Copy the repository root `.env.example` to `.env` and set `ESP32_BASE_URL` to the ESP32's current reachable base URL. Do not commit the local `.env`.
3. Optionally set `CAMERA_BASE_URL` to the mounted phone camera service and `AI_BASE_URL` to the existing FastAPI inference service.
4. From `dashboard/`, run `npm install`, then `npm run dev`.
5. Open the Vite URL printed in the terminal. The laptop backend listens on `BACKEND_PORT` (default 8000); Vite proxies API requests to it.

For a production build, run `npm run build`, then `npm start` from `dashboard/`. The backend serves `dist/` on the configured port.

## Gateway routes

- `GET /api/rover/status` proxies the ESP32 `/status` endpoint.
- `GET /api/rover/sensors` proxies `/sensorData` and `/status` as one snapshot.
- `POST /api/rover/control` with `{ "action": "forward|backward|left|right|stop|probe_insert|probe_retract|probe_stop" }` proxies existing hardware routes.
- `POST /api/rover/speed` with `{ "speed": 0..255 }` proxies `/speed?value=...`.
- `GET /api/vision/latest` reads the optional configured AI service.
- `/media/camera` and `/media/latest-image` proxy optional camera/AI media to the browser.

Device requests use a connection timeout and one short retry. Failed telemetry requests return an offline response with no sensor snapshot; the client clears prior values. The root ESP32 dashboard route now returns a small gateway message; its sensor, status, drive, speed, probe and Wi-Fi setup APIs remain available.

Camera, weather, map tiles, historical rover samples, and mission duration/distance require services/data the existing ESP32 API does not provide. The interface labels those capabilities unavailable rather than fabricating values. The robotic-arm UI uses the existing probe motor routes.
