# AGRI-X — AI Smart Agricultural Rover

AGRI-X is a field-monitoring rover prototype developed for Smart India Hackathon. It combines ESP32-based rover control and soil sensing with a local web dashboard, an AI crop-disease service, and a Flutter mobile application.

## Prototype at a glance

| Layer | What it does |
| --- | --- |
| ESP32 rover firmware | Drives the rover and probe motor; reads DHT, GPS, and NPK/soil data; hosts a Async web dashboard. |
| Soil intelligence | Interprets NPK, pH, EC, moisture, and temperature readings to produce crop and fertilizer guidance. |
| AI vision | Classifies crop-leaf disease images and returns treatment recommendations through a FastAPI service. |
| Mobile app | Flutter interface for live camera, scan results, history, map, and rover interaction. |

## Repository map

```text
.
├── src/                  ESP32 rover firmware implementation
├── include/              Firmware headers, pins, sensors, dashboard assets
├── platformio.ini        Primary PlatformIO configuration (ESP32 DevKit)
├── AI/                   FastAPI disease-detection service and Flutter app
├── Mini_project_AI/      Soil-analysis dashboard and reference ESP32 firmware
└── docs/                 Rover command reference
```

## Hardware used

- ESP32 DevKit V1
- Motor driver and DC motors for rover movement
- DHT temperature/humidity sensor
- GPS receiver
- RS485 NPK soil sensor (moisture, temperature, pH, N, P, K)
- Motorised soil probe

## Run the primary ESP32 firmware

1. Install [PlatformIO](https://platformio.org/) in VS Code.
2. Open this repository folder.
3. Connect the ESP32 DevKit and select its serial port.
4. Build and upload:

   ```bash
   pio run
   pio run -t upload
   pio device monitor -b 115200
   ```

The PlatformIO configuration automatically installs the Arduino dependencies listed in `platformio.ini`.

## Optional software modules

### AI disease-detection service

From `AI/`, install the Python packages required by `api.py` (FastAPI, Uvicorn, Ultralytics, OpenCV, and Pillow) and provide a trained model at:

```text
runs/classify/models/tomato_classifier/weights/best.pt
```

Then run the FastAPI application with Uvicorn. Model weights and datasets are intentionally excluded from Git so the repository stays lightweight; use your own trained model or download it from your project storage.

### Flutter application

From `AI/agri_x/`:

```bash
flutter pub get
flutter run
```

Set the API and rover addresses for your local network before deployment.

### Soil-analysis dashboard

From `Mini_project_AI/`:

```bash
pip install -r requirements.txt
python app.py
```

For the companion ESP32 project, copy `Mini_project_AI/esp32_firmware/CONFIG_TEMPLATE.h` to `config.h` and enter your own Wi-Fi and backend values. The real configuration file is deliberately ignored.

## Security and repository policy

This public repository contains source code and documentation only. Wi-Fi credentials, local IP-specific configuration, Python environments, build folders, datasets, and trained model weights are excluded through `.gitignore`. Never commit credentials or a production model without reviewing it first.

## Team note

This repository is organised to help reviewers trace the complete prototype—from sensing and rover control to field analytics, AI detection, and the mobile experience.
