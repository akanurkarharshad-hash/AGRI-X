import { config } from '../config.ts';

export class DeviceRequestError extends Error {
  constructor(message: string) { super(message); this.name = 'DeviceRequestError'; }
}

export async function requestEsp32(route: string, retry = true, allowPlainText = false): Promise<unknown> {
  if (!config.esp32BaseUrl) throw new DeviceRequestError('ESP32_BASE_URL is not configured');
  const maxAttempts = retry ? 2 : 1;
  let lastError = 'ESP32 request failed';
  for (let attempt = 0; attempt < maxAttempts; attempt += 1) {
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), config.requestTimeoutMs);
    try {
      const response = await fetch(`${config.esp32BaseUrl}${route}`, { signal: controller.signal, cache: 'no-store' });
      if (!response.ok) throw new DeviceRequestError(`ESP32 returned HTTP ${response.status}`);
      const text = await response.text();
      try { return JSON.parse(text) as unknown; }
      catch {
        if (allowPlainText) return text;
        throw new DeviceRequestError(`${route} returned malformed JSON`);
      }
    } catch (error) {
      lastError = error instanceof Error && error.name === 'AbortError' ? `ESP32 request timed out after ${config.requestTimeoutMs} ms` : (error instanceof Error ? error.message : 'ESP32 request failed');
      if (attempt + 1 < maxAttempts) await new Promise((resolve) => setTimeout(resolve, config.retryDelayMs));
    } finally { clearTimeout(timer); }
  }
  throw new DeviceRequestError(lastError);
}
