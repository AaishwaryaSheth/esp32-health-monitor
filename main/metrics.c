#include "metrics.h"
#include "led.h"

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_wifi.h"

// CPU load is derived from the FreeRTOS idle task run-time counters.
// We snapshot idle run-time per core each call and compare to the elapsed
// wall-clock to estimate how busy each core was since the previous sample.

#define NUM_CORES 2

static uint64_t s_prev_idle_runtime[NUM_CORES];
static uint64_t s_prev_total_time;

// Returns idle run-time counters per core via the run-time stats table.
static void read_idle_runtime(uint64_t out[NUM_CORES])
{
    out[0] = 0;
    out[1] = 0;

#if (configGENERATE_RUN_TIME_STATS == 1)
    UBaseType_t n = uxTaskGetNumberOfTasks();
    TaskStatus_t *arr = pvPortMalloc(n * sizeof(TaskStatus_t));
    if (!arr) {
        return;
    }
    uint32_t total;
    n = uxTaskGetSystemState(arr, n, &total);
    for (UBaseType_t i = 0; i < n; i++) {
        // IDLE task names are "IDLE0" / "IDLE1" with core-id enabled.
        if (strncmp(arr[i].pcTaskName, "IDLE", 4) == 0) {
            BaseType_t core = arr[i].xCoreID;
            if (core >= 0 && core < NUM_CORES) {
                out[core] = arr[i].ulRunTimeCounter;
            }
        }
    }
    vPortFree(arr);
#endif
}

static void compute_cpu_load(float load[NUM_CORES])
{
    uint64_t idle_now[NUM_CORES];
    read_idle_runtime(idle_now);

    uint64_t now = (uint64_t)esp_timer_get_time();
    uint64_t elapsed = now - s_prev_total_time;

    for (int c = 0; c < NUM_CORES; c++) {
        uint64_t idle_delta = idle_now[c] - s_prev_idle_runtime[c];
        float busy = 0.0f;
        if (elapsed > 0) {
            // run-time counter shares the same time base (esp_timer, us).
            float idle_ratio = (float)idle_delta / (float)elapsed;
            busy = (1.0f - idle_ratio) * 100.0f;
            if (busy < 0.0f) busy = 0.0f;
            if (busy > 100.0f) busy = 100.0f;
        }
        load[c] = busy;
        s_prev_idle_runtime[c] = idle_now[c];
    }
    s_prev_total_time = now;
}

size_t metrics_build_json(char *buf, size_t buf_len)
{
    uint32_t uptime_s = (uint32_t)(esp_timer_get_time() / 1000000ULL);
    uint32_t free_heap = (uint32_t)esp_get_free_heap_size();
    uint32_t min_free = (uint32_t)esp_get_minimum_free_heap_size();

    float load[NUM_CORES];
    compute_cpu_load(load);

    int rssi = 0;
    wifi_ap_record_t ap;
    if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) {
        rssi = ap.rssi;
    }

    int written = snprintf(
        buf, buf_len,
        "{\"uptime_s\":%u,\"free_heap\":%u,\"min_free_heap\":%u,"
        "\"cpu_load\":[%.1f,%.1f],\"rssi\":%d,\"led\":\"%s\"}",
        (unsigned)uptime_s, (unsigned)free_heap, (unsigned)min_free,
        load[0], load[1], rssi, led_get() ? "on" : "off");

    if (written < 0 || (size_t)written >= buf_len) {
        return 0;
    }
    return (size_t)written;
}
