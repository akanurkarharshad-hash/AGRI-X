import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import dotenv from 'dotenv';

dotenv.config({ path: '../.env' });
dotenv.config();
const backendPort = Number(process.env.BACKEND_PORT || 8000);

export default defineConfig({
  plugins: [react()],
  define: {
    'import.meta.env.VITE_CAMERA_CONFIGURED': JSON.stringify(Boolean(process.env.CAMERA_BASE_URL?.trim())),
  },
  server: { proxy: { '/api': `http://localhost:${backendPort}`, '/media': `http://localhost:${backendPort}` } },
});
