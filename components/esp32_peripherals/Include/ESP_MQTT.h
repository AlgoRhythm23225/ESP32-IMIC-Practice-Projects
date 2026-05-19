#include "mqtt_client.h"
#include "cJSON.h"

#define MQTT_TOPIC_WEATHER  "esp32_home/weather"  

extern esp_mqtt_client_handle_t client;

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void mqtt_app_start(void);
void parse_weather_data(const char *json_string);