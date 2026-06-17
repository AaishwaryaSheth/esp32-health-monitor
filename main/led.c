#include "led.h"
#include "app_config.h"
#include "driver/gpio.h"

static bool s_led_state = false;

void led_init(void)
{
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
    led_set(false);
}

void led_set(bool on)
{
    s_led_state = on;
    gpio_set_level(LED_GPIO, on ? 1 : 0);
}

bool led_toggle(void)
{
    led_set(!s_led_state);
    return s_led_state;
}

bool led_get(void)
{
    return s_led_state;
}
