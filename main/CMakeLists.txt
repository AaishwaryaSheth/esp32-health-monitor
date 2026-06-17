idf_component_register(
    SRCS
        "main.c"
        "wifi.c"
        "mqtt_client_task.c"
        "metrics.c"
        "led.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "certs/isrg_root_x1.pem"
)
