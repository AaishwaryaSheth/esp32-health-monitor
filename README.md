# ESP32 Health Monitor

Real-time ESP32 system-health dashboard over MQTT. The board runs **FreeRTOS
(ESP-IDF)** and publishes live metrics to a **HiveMQ Cloud** broker; a static
dashboard hosted on **GitHub Pages** subscribes over secure WebSocket and
renders them live. The LED is **bidirectional** — toggle it from the browser.

> **Is it real-time or just a demo?** Genuinely real-time. The ESP32 publishes
> every second; the dashboard updates in ~100–300 ms via MQTT-over-WSS. The LED
> button publishes a command the board acts on immediately. GitHub Pages only
> serves static files — all live data flows through HiveMQ.

## Metrics shown

| Metric | Source | Live? |
|---|---|---|
| Free heap / min free heap | `esp_get_free_heap_size()` | yes |
| CPU load per core | FreeRTOS idle run-time stats | yes |
| Uptime | `esp_timer` | yes |
| WiFi RSSI | `esp_wifi_sta_get_ap_info()` | yes |
| LED status + control | GPIO2, bidirectional MQTT | yes |

## Architecture

```
ESP32 (FreeRTOS)            HiveMQ Cloud (TLS)          GitHub Pages
  metrics task ──publish──► health/<id>/telemetry ──► dashboard (wss:8884)
  led task     ◄─subscribe─ health/<id>/led/set   ◄── Toggle button
               ──publish──► health/<id>/led/state ──► LED indicator
```

## 1. Prerequisites

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/) v5.x installed
- A free [HiveMQ Cloud](https://www.hivemq.com/mqtt-cloud-broker/) cluster
  (create a username/password under **Access Management**)
- An ESP32 dev board (onboard LED on GPIO2)

## 2. Configure

Edit `main/app_config.h`:

- `WIFI_SSID` / `WIFI_PASSWORD`
- `MQTT_BROKER_URI` → `mqtts://YOUR-CLUSTER.s1.eu.hivemq.cloud:8883`
- `MQTT_USERNAME` / `MQTT_PASSWORD`
- `DEVICE_ID` (must match the Device ID you enter in the dashboard)

> The HiveMQ Cloud server certificate is signed by Let's Encrypt. The root CA
> (`main/certs/isrg_root_x1.pem`) is embedded into the firmware for TLS
> verification — no action needed.

> **Security:** don't commit real credentials to a public repo. For production
> move secrets into `main/app_secrets.h` (git-ignored) or use `idf.py menuconfig`.

## 3. Build & flash

```bash
idf.py set-target esp32
idf.py build
idf.py -p <PORT> flash monitor
```

Watch the log for `connected to broker`.

## 4. Deploy the dashboard

1. Push this repo to GitHub.
2. **Settings → Pages → Source: GitHub Actions**. The included
   `.github/workflows/pages.yml` publishes `docs/` automatically.
3. Open the published URL, expand **Connection settings**, enter your HiveMQ
   Cloud host (no protocol/port), username, password, and Device ID, then
   **Connect**.

You can also just open `docs/index.html` locally in a browser — it connects to
the same cloud broker.

## Project layout

```
esp32-health-monitor/
├─ main/
│  ├─ main.c                 # app entry: init led, wifi, mqtt
│  ├─ app_config.h           # WiFi / HiveMQ / topic / pin config
│  ├─ wifi.c/.h              # station-mode WiFi connect
│  ├─ mqtt_client_task.c/.h  # TLS MQTT + telemetry task + LED command handling
│  ├─ metrics.c/.h           # heap / CPU load / uptime / RSSI -> JSON
│  ├─ led.c/.h               # GPIO2 LED status + control
│  └─ certs/isrg_root_x1.pem # embedded root CA for HiveMQ Cloud TLS
├─ docs/index.html           # GitHub Pages dashboard (MQTT.js over WSS)
└─ .github/workflows/pages.yml
```
