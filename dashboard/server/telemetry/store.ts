import { mkdir, readFile, appendFile } from 'node:fs/promises';
import path from 'node:path';
import { config } from '../config.ts';
import { log } from '../logger.ts';
import type { CanonicalTelemetry } from './types.ts';

export async function loadLatest(): Promise<CanonicalTelemetry | null> {
  try {
    const content = await readFile(config.historyFile, 'utf8');
    const lastLine = content.trimEnd().split('\n').at(-1);
    if (!lastLine) return null;
    return JSON.parse(lastLine) as CanonicalTelemetry;
  } catch (error) {
    if ((error as NodeJS.ErrnoException).code !== 'ENOENT') log('error', 'history_load_failed', { message: String(error) });
    return null;
  }
}

export async function appendTelemetry(sample: CanonicalTelemetry): Promise<void> {
  await mkdir(path.dirname(config.historyFile), { recursive: true });
  await appendFile(config.historyFile, `${JSON.stringify(sample)}\n`, 'utf8');
}

export async function readHistory(limit: number, before?: string): Promise<CanonicalTelemetry[]> {
  try {
    const content = await readFile(config.historyFile, 'utf8');
    const rows: CanonicalTelemetry[] = [];
    for (const line of content.split('\n')) {
      if (!line.trim()) continue;
      try {
        const item = JSON.parse(line) as CanonicalTelemetry;
        if (!before || item.timestamp < before) rows.push(item);
      } catch { log('warn', 'history_row_malformed'); }
    }
    return rows.slice(-limit).reverse();
  } catch (error) {
    if ((error as NodeJS.ErrnoException).code !== 'ENOENT') throw error;
    return [];
  }
}
