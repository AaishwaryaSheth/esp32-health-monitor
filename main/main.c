#include "app_config.h"
#include "led.h"
#include "wifi.h"
#include "mqtt_client_task.h"

#include "esp_log.h"

static const char *TAG = "app";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Health Monitor starting (device=%s)", DEVICE_ID);

    led_init();
    wifi_init_sta();   // blocks until WiFi connected (or gives up)
    mqtt_start();      // connects to HiveMQ Cloud, starts telemetry task

    // app_main returns; FreeRTOS tasks keep running.
}
