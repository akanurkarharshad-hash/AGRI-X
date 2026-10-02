type Level = 'info' | 'warn' | 'error';

export function log(level: Level, event: string, details: Record<string, unknown> = {}): void {
  process.stdout.write(`${JSON.stringify({ timestamp: new Date().toISOString(), level, service: 'agri-x-gateway', event, ...details })}\n`);
}
