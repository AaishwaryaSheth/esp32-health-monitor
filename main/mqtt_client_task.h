#pragma once

// Start the MQTT client (connects to HiveMQ Cloud over TLS), subscribes to
// the LED command topic, and spawns a task that publishes telemetry every
// TELEMETRY_PERIOD_MS. Call after wifi_init_sta() returns connected.
void mqtt_start(void);
