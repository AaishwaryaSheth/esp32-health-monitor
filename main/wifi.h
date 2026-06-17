#pragma once

// Connect to WiFi in station mode using credentials from app_config.h.
// Blocks (with retries) until connected, then returns. Logs progress.
void wifi_init_sta(void);
