#include "mqtt_client_task.h"
#include "app_config.h"
#include "metrics.h"
#include "led.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "mqtt";

// HiveMQ Cloud presents a Let's Encrypt chain rooted at ISRG Root X1.
// Embedded via EMBED_TXTFILES in main/CMakeLists.txt.
extern const uint8_t isrg_root_x1_pem_start[] asm("_binary_isrg_root_x1_pem_start");
extern const uint8_t isrg_root_x1_pem_end[]   asm("_binary_isrg_root_x1_pem_end");

static esp_mqtt_client_handle_t s_client;
static volatile bool s_connected = false;

static void publish_led_state(void)
{
    if (!s_connected) return;
    const char *state = led_get() ? "on" : "off";
    esp_mqtt_client_publish(s_client, TOPIC_LED_STATE, state, 0, 1, true);
}

static void handle_led_cmd(const char *data, int len)
{
    if (len == 2 && strncmp(data, "on", 2) == 0) {
        led_set(true);
    } else if (len == 3 && strncmp(data, "off", 3) == 0) {
        led_set(false);
    } else if (len == 6 && strncmp(data, "toggle", 6) == 0) {
        led_toggle();
    } else {
        ESP_LOGW(TAG, "unknown LED command (%d bytes)", len);
        return;
    }
    ESP_LOGI(TAG, "LED -> %s", led_get() ? "on" : "off");
    publish_led_state();
}

static void mqtt_event_handler(void *args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "connected to broker");
        s_connected = true;
        esp_mqtt_client_subscribe(s_client, TOPIC_LED_CMD, 1);
        publish_led_state();
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "disconnected");
        s_connected = false;
        break;
    case MQTT_EVENT_DATA:
        if (event->topic_len == (int)strlen(TOPIC_LED_CMD) &&
            strncmp(event->topic, TOPIC_LED_CMD, event->topic_len) == 0) {
            handle_led_cmd(event->data, event->data_len);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "mqtt error");
        break;
    default:
        break;
    }
}

static void telemetry_task(void *arg)
{
    char json[256];
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        if (s_connected) {
            size_t n = metrics_build_json(json, sizeof(json));
            if (n > 0) {
                esp_mqtt_client_publish(s_client, TOPIC_TELEMETRY, json, n, 0, false);
            }
        }
        vTaskDelayUntil(&last, pdMS_TO_TICKS(TELEMETRY_PERIOD_MS));
    }
}

void mqtt_start(void)
{
    esp_mqtt_client_config_t cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URI,
            .verification.certificate = (const char *)isrg_root_x1_pem_start,
        },
        .credentials = {
            .username = MQTT_USERNAME,
            .authentication.password = MQTT_PASSWORD,
            .client_id = DEVICE_ID,
        },
        .session = {
            // Last Will: dashboard can detect the board dropping offline.
            .last_will = {
                .topic = TOPIC_LED_STATE,
                .msg = "offline",
                .qos = 1,
                .retain = true,
            },
        },
    };

    s_client = esp_mqtt_client_init(&cfg);
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_client);

    xTaskCreate(telemetry_task, "telemetry", 4096, NULL, 5, NULL);
}
