// from: C
#include <string.h>
// from: ESP-IDF
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
// from: "annoying_default_funcs" folder
#include "nvs_init_in_main.h"
// project
#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "wifi_sta_ap.h"
esp_mqtt_client_handle_t client = NULL;
const char *TAG = "[MQTT]";

#define THINGNAME "Esp32"

// Amazone ROOT CA
extern const uint8_t root_ca_pem_start[] asm("_binary_AmazonRootCA1_pem_start");
extern const uint8_t root_ca_pem_end[] asm("_binary_AmazonRootCA1_pem_end");

extern const uint8_t device_pem_crt_start[] asm("_binary_Device_Certificate_crt_start");
extern const uint8_t device_pem_crt_end[] asm("_binary_Device_Certificate_crt_end");

extern const uint8_t private_pem_key_start[] asm("_binary_Private_Key_key_start");
extern const uint8_t private_pem_key_end[] asm("_binary_Private_Key_key_end");
#define MQTT_TOPIC_WEATHER  "esp32_home/weather"  

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        // When connect successfully, subcribe to a topic to inform the online status
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");

            // subcribe topic
            esp_mqtt_client_subscribe(client, "esp32/led", 1);
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_WEATHER, 1);

            // try publish
            esp_mqtt_client_publish(client, "esp32/led", "online", 0, 0, 0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT disconnected");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "Subcribed");
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Published");
            break;
        
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
            if (strncmp(event->data, "ON", event->data_len) == 0) {
                gpio_set_level(LED_PIN, 1);
            }
            else if (strncmp(event->data, "OFF", event->data_len) == 0) {
                gpio_set_level(LED_PIN, 0);          
            }
            break;

        default:
            break;
    }
}

void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtts://a1974gzwyohsln-ats.iot.us-east-1.amazonaws.com:8883",
        .broker.verification.certificate = (const char *)root_ca_pem_start,
        .credentials.authentication.certificate = (const char *)device_pem_crt_start,
        .credentials.authentication.key = (const char *)private_pem_key_start,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    esp_mqtt_client_start(client);
}

#define WEATHER_API_URL     "http://api.openweathermap.org/data/2.5/weather?q=Hanoi&appid=bf7911260e33a6b7ebd42f893fbe368a&units=metric"
char response_data[1024];
int response_len = 0;
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
                memcpy(response_data + response_len, evt->data, evt->data_len);
                response_len += evt->data_len;
            }
            break;

        case HTTP_EVENT_ON_FINISH:
            response_data[response_len] = '\0';
            break;

        default:
            break;
    }
    return ESP_OK;
}

void parse_weather_data(const char *json_string) {
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL) {
        ESP_LOGE("[PARSE]", "Can't parse JSON");
        return;
    }

    cJSON *main_obj = cJSON_GetObjectItem(root, "main");
    if (main_obj) {
        cJSON *temp = cJSON_GetObjectItem(main_obj, "temp");
        cJSON *humidity = cJSON_GetObjectItem(main_obj, "humidity");

        if (cJSON_IsNumber(temp) && cJSON_IsNumber(humidity)) {
            ESP_LOGI("[API]", "Temperature: %.2f ºC", temp->valuedouble);
            ESP_LOGI("[API]", "Humidity: %d %%", humidity->valueint);

            char message[64];
            // snprintf(message, sizeof(message), "{\"temp\":%.2f, \"hum\":%d}", temp->valuedouble, humidity->valueint);
            snprintf(message, sizeof(message), "Temperature: %.2fºC\nHumidity: %d%%", temp->valuedouble, humidity->valueint);
            esp_mqtt_client_publish(client, "esp32_home/weather", message, 0, 1, 0);
        }
    }
    cJSON_Delete(root);
}

void weather_task(void *pvParameters) {
    while (1) {
        if (client != NULL) {
            esp_http_client_config_t config = {
                .url = WEATHER_API_URL,
                .method = HTTP_METHOD_GET,
                .event_handler = _http_event_handler,
            };
            esp_http_client_handle_t http_client = esp_http_client_init(&config);
            response_len = 0;
            
            esp_err_t err = esp_http_client_perform(http_client);

            if (err == ESP_OK) {
                ESP_LOGI("[HTTP]", "HTTP GET Status = %d", esp_http_client_get_status_code(http_client));
                parse_weather_data(response_data);
                ESP_LOGI("[MQTT]", "Sent MQTT, free heap: %d", esp_get_free_heap_size());
                if (esp_get_free_heap_size() < 100000) {
                    esp_restart();
                }
            } else {
                ESP_LOGE("[HTTP]", "HTTP GET request failed: %s", esp_err_to_name(err));
                ESP_LOGE("[HTTP]", "HTTP GET Status = %d", esp_http_client_get_status_code(http_client));           
            }   
            esp_http_client_cleanup(http_client);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void) {
    nvs_flash_init_in_main();

    wifi_init_sta();
    WIFI_WAIT_CONNECT(wifi_event_group);

    mqtt_app_start();
    xTaskCreate(weather_task, "weather_task", 8192, NULL, 5, NULL);

}
