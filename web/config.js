// When hosting this folder on a laptop, set ESP32_HOST to the rover's IP.
// Keep WS_SHARED_KEY matched with TimingConfig::WEBSOCKET_SHARED_KEY in firmware.
window.AGRIX_CONFIG = {
    ESP32_HOST: "192.168.4.1",
    WS_SHARED_KEY: "agrix-change-this-key"
};
