#pragma once

// ===================== APP CONFIG =====================
// Real credentials (WiFi + HiveMQ) live in app_secrets.h, which is git-ignored.
// Copy app_secrets.h.example -> app_secrets.h and fill in your values.
#include "app_secrets.h"

// Non-secret configuration below is safe to commit.

// Unique id for this device. Used in topics so multiple boards don't collide.
#define DEVICE_ID        "esp32-health-01"

// --- Topics ---
// Telemetry published by the ESP32 (dashboard subscribes).
#define TOPIC_TELEMETRY  "health/" DEVICE_ID "/telemetry"
// LED command sent by the dashboard (ESP32 subscribes). Payload: "on" / "off" / "toggle".
#define TOPIC_LED_CMD    "health/" DEVICE_ID "/led/set"
// LED state echoed back by the ESP32 (dashboard subscribes). Payload: "on" / "off".
#define TOPIC_LED_STATE  "health/" DEVICE_ID "/led/state"

// --- Hardware ---
#define LED_GPIO         2        // Onboard LED on most ESP32 DevKitC boards

// --- Timing ---
#define TELEMETRY_PERIOD_MS  1000 // publish metrics once per second
