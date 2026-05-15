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
// #include "mqtt_client.h"
// #include "esp_log.h"
// #include "esp_http_client.h"
// #include "cJSON.h"
// #include "esp_crt_bundle.h"
// #include "wifi_sta_ap.h"
// esp_mqtt_client_handle_t client = NULL;
// const char *TAG = "[MQTT]";

// #define THINGNAME "Esp32"

// // Amazone ROOT CA
// extern const uint8_t root_ca_pem_start[] asm("_binary_AmazonRootCA1_pem_start");
// extern const uint8_t root_ca_pem_end[] asm("_binary_AmazonRootCA1_pem_end");

// extern const uint8_t device_pem_crt_start[] asm("_binary_Device_Certificate_crt_start");
// extern const uint8_t device_pem_crt_end[] asm("_binary_Device_Certificate_crt_end");

// extern const uint8_t private_pem_key_start[] asm("_binary_Private_Key_key_start");
// extern const uint8_t private_pem_key_end[] asm("_binary_Private_Key_key_end");
// #define MQTT_TOPIC_WEATHER  "esp32_home/weather"  

// bool is_stop_mqtt = false;
// void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
//     esp_mqtt_event_handle_t event = event_data;

//     switch ((esp_mqtt_event_id_t)event_id) {
//         // When connect successfully, subcribe to a topic to inform the online status
//         case MQTT_EVENT_CONNECTED:
//             ESP_LOGI(TAG, "MQTT connected");

//             // subcribe topic
//             esp_mqtt_client_subscribe(client, "esp32/led", 1);
//             esp_mqtt_client_subscribe(client, MQTT_TOPIC_WEATHER, 1);

//             // try publish
//             esp_mqtt_client_publish(client, "esp32/led", "online", 0, 0, 0);
//             break;

//         case MQTT_EVENT_DISCONNECTED:
//             ESP_LOGI(TAG, "MQTT disconnected");
//             break;

//         case MQTT_EVENT_SUBSCRIBED:
//             ESP_LOGI(TAG, "Subcribed");
//             break;

//         case MQTT_EVENT_PUBLISHED:
//             ESP_LOGI(TAG, "Published");
//             break;
        
//         case MQTT_EVENT_DATA:
//             ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
//             ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
//             if (strncmp(event->data, "ON", event->data_len) == 0) {
//                 gpio_set_level(LED_PIN, 1);
//             } 
//             else if (strncmp(event->data, "OFF", event->data_len) == 0) {
//                 gpio_set_level(LED_PIN, 0);          
//             }
//             else if (strncmp(event->data, "Disconnect", event->data_len) == 0) {
//                 is_stop_mqtt = true;
//             }
//             break;

//         default:
//             break;
//     }
// }

// void mqtt_app_start(void) {
//     esp_mqtt_client_config_t mqtt_cfg = {
//         .broker.address.uri = "mqtts://a1974gzwyohsln-ats.iot.us-east-1.amazonaws.com:8883",
//         .broker.verification.certificate = (const char *)root_ca_pem_start,
//         .credentials.authentication.certificate = (const char *)device_pem_crt_start,
//         .credentials.authentication.key = (const char *)private_pem_key_start,
//     };

//     client = esp_mqtt_client_init(&mqtt_cfg);

//     esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

//     esp_mqtt_client_start(client);
// }

// #define WEATHER_API_URL     "http://api.openweathermap.org/data/2.5/weather?q=Hanoi&appid=bf7911260e33a6b7ebd42f893fbe368a&units=metric"
// char response_data[1024];
// int response_len = 0;
// esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
//     switch (evt->event_id) {
//         case HTTP_EVENT_ON_DATA:
//             if (!esp_http_client_is_chunked_response(evt->client)) {
//                 memcpy(response_data + response_len, evt->data, evt->data_len);
//                 response_len += evt->data_len;
//             }
//             break;

//         case HTTP_EVENT_ON_FINISH:
//             response_data[response_len] = '\0';
//             break;

//         default:
//             break;
//     }
//     return ESP_OK;
// }

// void parse_weather_data(const char *json_string) {
//     cJSON *root = cJSON_Parse(json_string);
//     if (root == NULL) {
//         ESP_LOGE("[PARSE]", "Can't parse JSON");
//         return;
//     }

//     cJSON *main_obj = cJSON_GetObjectItem(root, "main");
//     if (main_obj) {
//         cJSON *temp = cJSON_GetObjectItem(main_obj, "temp");
//         cJSON *humidity = cJSON_GetObjectItem(main_obj, "humidity");

//         if (cJSON_IsNumber(temp) && cJSON_IsNumber(humidity)) {
//             ESP_LOGI("[API]", "Temperature: %.2f ºC", temp->valuedouble);
//             ESP_LOGI("[API]", "Humidity: %d %%", humidity->valueint);

//             char message[64];
//             // snprintf(message, sizeof(message), "{\"temp\":%.2f, \"hum\":%d}", temp->valuedouble, humidity->valueint);
//             snprintf(message, sizeof(message), "Temperature: %.2fºC\nHumidity: %d%%", temp->valuedouble, humidity->valueint);
//             esp_mqtt_client_publish(client, "esp32_home/weather", message, 0, 1, 0);
//         }
//     }
//     cJSON_Delete(root);
// }


// void weather_task(void *pvParameters) {
//     while (1) {
//         if (is_stop_mqtt) {
//             esp_mqtt_client_stop(client);
//             client = NULL;
//         }

//         if (client != NULL) {
//             esp_http_client_config_t config = {
//                 .url = WEATHER_API_URL,
//                 .method = HTTP_METHOD_GET,
//                 .event_handler = _http_event_handler,
//             };
//             esp_http_client_handle_t http_client = esp_http_client_init(&config);
//             response_len = 0;
            
//             esp_err_t err = esp_http_client_perform(http_client);

//             if (err == ESP_OK) {
//                 ESP_LOGI("[HTTP]", "HTTP GET Status = %d", esp_http_client_get_status_code(http_client));
//                 parse_weather_data(response_data);
//                 ESP_LOGI("[MQTT]", "Sent MQTT, free heap: %d", esp_get_free_heap_size());
//                 if (esp_get_free_heap_size() < 100000) {
//                     esp_restart();
//                 }
//             } else {
//                 ESP_LOGE("[HTTP]", "HTTP GET request failed: %s", esp_err_to_name(err));
//                 ESP_LOGE("[HTTP]", "HTTP GET Status = %d", esp_http_client_get_status_code(http_client));           
//             }   
//             esp_http_client_cleanup(http_client);
//         }
//         vTaskDelay(pdMS_TO_TICKS(5000));
//     }
// }

#include "driver/gpio.h"
#include "driver/spi_master.h"

#define PIN_NUM_MOSI    23
#define PIN_NUM_CLK     18
#define PIN_NUM_CS      5
#define PIN_NUM_DC      2
#define PIN_NUM_RST     4

spi_device_handle_t tft_spi = NULL;

void gpio_init_tft() {
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
}

void bus_init() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_NUM_CLK,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
    };

    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 20 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7,
    };

    spi_bus_add_device(SPI2_HOST, &devcfg, &tft_spi);
}

void lcd_cmd(uint8_t cmd) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    t.length = 8;
    t.tx_buffer = &cmd;

    gpio_set_level(PIN_NUM_DC, 0);

    spi_device_transmit(tft_spi, &t);
}

void lcd_data(const uint8_t *data, int len) {
    if (len == 0) {
        return;
    }

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    t.length = len * 8;
    t.tx_buffer = data;

    gpio_set_level(PIN_NUM_DC, 1);

    spi_device_transmit(tft_spi, &t);
}

void lcd_reset() {
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void st7735_init() {
    gpio_init_tft();

    lcd_reset();

    // SWRESET 
    lcd_cmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(150));
    
    // SLPOUT
    lcd_cmd(0x11);
    vTaskDelay(pdMS_TO_TICKS(150));

    // COLMOD = RGB565
    lcd_cmd(0x3A);

    uint8_t data = 0x05;

    lcd_data(&data, 1);

    // DISPON
    lcd_cmd(0x29);
}

void st7735_set_window(
    uint16_t x0,
    uint16_t y0,
    uint16_t x1,
    uint16_t y1)
{
    uint8_t data[4];

    // CASET
    lcd_cmd(0x2A);

    data[0] = x0 >> 8;
    data[1] = x0 & 0xFF;
    data[2] = x1 >> 8;
    data[3] = x1 & 0xFF;

    lcd_data(data, 4);

    // RASET
    lcd_cmd(0x2B);
    data[0] = y0 >> 8;
    data[1] = y0 & 0xFF;
    data[2] = y1 >> 8;
    data[3] = y1 & 0xFF;

    lcd_data(data, 4);

    // RAMWR
    lcd_cmd(0x2C);
} 

typedef struct color {
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
}color_t;

uint16_t RGB_convert(uint8_t R, uint8_t G, uint8_t B) {
    color_t color = {R, G, B};
    return (color.Red >> 3) | (color.Green >> 2) << 5 | (color.Blue >> 3) << 11;
}

#define x1 103 // ofset = 24
#define y1 159
void st7735_fill(uint8_t R, uint8_t G, uint8_t B) {
    st7735_set_window(24, 0, x1, y1);

    uint8_t line[80 * 2];

    uint16_t color = RGB_convert(R, G, B);
    for (int i = 0; i < 80; i++) {
        line[i * 2] = color >> 8;
        line[i * 2 + 1] = color & 0xFF;
    }

    for (int y = 0; y < 160; y++) {
        lcd_data(line, sizeof(line));
    }
}

void st7735_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x > x1 || y > y1) {
        return;
    }

    st7735_set_window(x, y, x, y);

    uint8_t data[2];
    data[0] = color >> 8;
    data[1] = color & 0xFF;

    lcd_data(data, 2);
}

void st7735_fill_rect(uint16_t x, uint16_t y,
                      uint16_t w, uint16_t h,
                      uint16_t color)
{
    if (x + w > x1 + 1) w = (x1 + 1) - x;
    if (y + h > y1 + 1) h = (y1 + 1) - y;

    st7735_set_window(x, y, x + w - 1, y + h - 1);

    uint8_t line[w * 2];

    for (int i = 0; i < w; i++) {
        line[i * 2] = color >> 8;
        line[i * 2 + 1] = color & 0xFF;
    }

    for (int i = 0; i < h; i++) {
        lcd_data(line, sizeof(line));
    }
}

void app_main(void) {
    nvs_flash_init_in_main();

    bus_init();
    st7735_init();
    st7735_fill(164, 119, 100);
    st7735_fill_rect(50, 50, 30, 50, RGB_convert(255, 0, 0));
}


// wifi_init_sta();
// WIFI_WAIT_CONNECT(wifi_event_group);

// mqtt_app_start();
// xTaskCreate(weather_task, "weather_task", 8192, NULL, 5, NULL);