# GrowTent

> ⚡ Smart, adaptive grow tent controller with real-time climate stabilization and OTA updates

GrowTent is a compact ESP32-based grow-tent controller that reads temperature, humidity and computes VPD (Vapor Pressure Deficit) using a BME280 sensor.

It provides a web interface (German/English), relay control, automation logic and integrates with Shelly devices. The system is designed for **stable long-term operation** and **minimal manual intervention**.

---

## 🚀 Features

- **ESP32 web interface** for live status and configuration
- **Adaptive sensor smoothing (EMA)**
  - dynamic smoothing based on real-time changes
  - fast response to environmental shifts (e.g. opening tent/window)
  - stable readings in steady conditions
- **Raw vs smoothed sensor values**
  - full transparency for debugging and tuning
- **Stable VPD calculation**
  - based on smoothed values
  - reduces oscillation in control loops
- **4x or 8x relay board support** (selectable in UI)
- **Relay scheduling** (minute-based within each hour)
- **Conditional relay logic** (e.g. based on light state)
- **Shelly integration**
  - Main power
  - Grow light
  - Humidifier
  - Heater
  - Fan (only on 8x boards)
  - Exthaust (only on 8x boards)
- **Heater control by temperature**
  - ESP relay or Shelly device
- **Irrigation support (only on 8x boards)** 
  - peristaltic pumps (relay 6–8)
- **Tank level monitoring (HC-SR04) (only on 8x boards)**
  - distance-based water level detection
  - usable for irrigation safety (empty tank protection)
- **Built-in log buffer**
- **History and averages**
- **OTA update from GitHub Release binary**
- **Designed for stability**
  - optimized for unattended operation

---

## 🌡️ Adaptive Climate Smoothing

The system uses an **adaptive exponential moving average (EMA)** to balance
stability and responsiveness of sensor readings.

### How it works

- Raw sensor values are read from the BME280
- The system calculates the difference between consecutive readings
- Based on this delta, a dynamic smoothing factor (`alpha`) is applied:
  - small changes → strong smoothing
  - large changes → fast reaction

### Example

- Stable environment → smooth values  
- Opening a window → immediate response  

### Benefits

- Reduced sensor noise  
- Stable VPD calculation  
- Faster reaction to real changes  
- Improved control behavior  

## Hardware

- ESP32 4x or 8x relay board (any compatible dev board)
- Adafruit BME280 (I2C at default address 0x76)
- 4x relay channels (driven by ESP32 GPIOs)
- Power supply appropriate for ESP32 and relays
- HC-SR04 sensor 

### Default sensor and relay configuration in code:
- BME I2C address: `0x76` (BME_ADDR)
- DS18B20 sensor pin 4
- Relay count: `4` (NUM_RELAYS)
- 4x Relay pins (in order): `{ 32, 33, 25, 26 }` 
- 8x Relay pins (in order): `{32, 33, 25, 26, 27, 14, 12, 13 }`
- only for 8x Relay board connect Relay 6, 7, 8 to a peristaltic pump for watering the pods
- Shelly devices (Gen1/Gen2/Gen3 depending on endpoint support)

### Wiring notes:
- Connect BME280 SDA / SCL to ESP32 SDA / SCL (Wire library pins)
- Make sure common ground between relays and ESP32
- Relays are initialized as OUTPUT and driven LOW = OFF by default

### Relay Boards
- 4-channel relay board
- 8-channel relay board

---

## Defaults & Constants

- AP SSID: `new growtent`
- AP password: `GT-12$34`
- Measurement interval: `30000 ms` (30 seconds)
- Default NTP server: `de.pool.ntp.org`
- Default timezone (POSIX-style): Western European Time, example:
  `WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00`
- Default language: `de` (German)
- Default theme: `light`
- Default units: `metric`
- Default time format: `24h`
- Default VPD targets (per phase): `{ 0.0, 0.8, 1.2, 1.4, 1.0 }`
- Phase names: `["", "Vegetative", "Flowering", "Drying"]`

Preferences keys stored in NVS:
- `ssid` / `password` (WiFi)
- `boxName`
- `lang`
- `theme`
- `unit`
- `timeFmt`
- `ntpSrv`
(Namespace: `growtent`)

---

## Web UI

The web frontend is embedded in program memory and served by the ESP32:
- Main page: `/` (index)
- Stylesheet: `/style.css`
- JavaScript: `/script.js`
- i18n manifest and language JSONs are served from memory
- Factory reset endpoint available via `POST /factory-reset` (form in UI)

UI highlights:
- Gauges for Temperature, Humidity and VPD
- Settings page for language, theme, date/time format, temperature unit, and NTP/timezone configuration
- Uses localStorage + Preferences for persistent settings across reloads
- Statuspage
 <img width="1702" height="1124" alt="image" src="https://github.com/user-attachments/assets/5439fcfe-d08a-42a5-b105-eb2148a9d6f3" />




Languages provided (i18n):
- German (`de`)
- English (`en`)

---

## VPD Calculation

VPD (kPa) is calculated from the measured temperature (°C) and relative humidity (%) using saturation vapor pressure (svp) approximation:

svp = 0.6108 * exp((17.27 * T) / (T + 237.3))
VPD = svp - (RH / 100) * svp

Values are updated by a background task and stored in:
- `lastTemperature`
- `lastHumidity`
- `lastVPD`

---

## Software / Libraries

- Arduino framework for ESP32
- Wire (I2C)
- Adafruit_BME280
- Preferences (NVS)
- Adafruit Sensor (sensor abstraction)

Make sure to install the Adafruit_BME280 and Adafruit_Sensor libraries before building.

---

## Project Structure

- `src/main.cpp`
Web routes, setup, loop, task startup
- `src/function.h`
Core logic (control, API builders, handlers)
- `src/runtime.h`
Preferences load and HTML placeholder rendering
- `src/java_script.h`
Frontend behavior (status polling, UI updates)
- `src/index_html.h`
Embedded web page template
- `src/style_css.h`
Embedded styles
- `src/task_Check_Sensor.h`
Sensor/control task
- `src/task_CheckShellyStatus.h`
Shelly poll task
- `src/task_Watering.h`
Irrigation task

---

## Build & Flash (quick)

These are general instructions — adapt to your toolchain (Arduino IDE, PlatformIO, Arduino CLI, etc.)

1. Install required libraries:
   - Adafruit_BME280
   - Adafruit_Sensor

2. Select the correct ESP32 board in your build environment.

3. Build and flash to your ESP32 as usual.

Example (PlatformIO):
- pio run --target upload

Example (Arduino CLI):
- arduino-cli compile --fqbn <fqbn> .
- arduino-cli upload -p <port> --fqbn <fqbn> .

---

## Configuration & Tuning

- WiFi: You can set the device to Station mode or keep it in AP mode for direct connection.
- NTP and timezone: Set the NTP server and timezone string via the web UI.
- Relay behavior: By default relays are set LOW at startup. Update logic in code to change behavior.

---

## API Endpoints (Connectors)

### Core
- `GET /api/state`
Unified state payload for frontend and external integrations (recommended primary endpoint).

### Control
- `POST /relay/{1..5}/toggle`
- `POST /pump/6/triggerPump10s`
- `POST /pump/7/triggerPump10s`
- `POST /pump/8/triggerPump10s`
- `POST /startWatering`
- `POST /pingTank`

### Shelly
- `POST /shelly/main/toggle`
- `POST /shelly/light/toggle`
- `POST /shelly/humidifier/toggle`
- `POST /shelly/heater/toggle`
- `POST /api/shelly/reset-energy`

### Scheduling / Settings
- `POST /api/relay/schedule/save-all`
- `POST /savesettings`
- `POST /saverunsettings`
- `POST /saveshellysettings`
- `POST /savemessagesettings`

### Logs
- `GET /api/logbuffer`
- `POST /api/logbuffer/clear`
- `GET /log`

### OTA
- `POST /api/ota/update`
Triggers OTA update from the configured firmware URL.

---

## ⚙️ Configuration

- WiFi: AP or Station mode
- NTP + timezone configurable via UI
- Relay logic configurable via UI

---

## Project Structure (high level)

- src/
  - main.cpp (setup, loop, initialization)
  - config.h (hardware and global configuration)
  - task_Check_Sensor.h (background sensor read + VPD logic)
  - index_html.h (web frontend HTML + i18n JSON)
  - java_script.h (client JS embedded)
  - style_css.h (client CSS embedded)
  - other .h / helper files

All web UI assets are embedded as string literals (PROGMEM) and served by the device.

---

## 🧪 Troubleshooting

- BME280 not found → check wiring & I2C address  
- Relays not switching → check GPIO + logic level  
- No UI → connect to AP:  
  - SSID: `new growtent`  
  - Password: `GT-12$34`

---

## Contributing

Contributions, bug reports and improvements welcome. Please open issues or pull requests describing the change and rationale.

---

## License

Add license information here (no license file present in the source snapshot). If you want to use a permissive license, consider adding an SPDX header and LICENSE file (MIT, Apache-2.0, etc.).

---
