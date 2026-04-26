#include "wifi_sta_ap.h"

// Handler for connect/disconnect event
static const char *TAG_SOFTAP = "wifi softAP";
void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG_SOFTAP, "station "MACSTR" join, AID=%d",
                MAC2STR(event->mac), event->aid);
        gpio_set_level(LED_PIN, 1);
                 
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG_SOFTAP, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
        gpio_set_level(LED_PIN, 0); 
    }
}

// WIFI SoftAP MODE
void wifi_init_softap()
{
    // I. KHỞI TẠO HỆ THỐNG
    ESP_ERROR_CHECK(esp_netif_init());                          // Khởi tạo TCP/IP stack (lwip)
    ESP_ERROR_CHECK(esp_event_loop_create_default());           // Tạo event loop
    esp_netif_create_default_wifi_ap();                         // SoftAP interface và cấp IP cho ESP32 (default)

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();        // Lấy cấu hình cho Wifi driver
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));                       // Khởi tạo Wifi driver

    // II. HANDLER CHO CÁC SỰ KIỆN
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    // III. CẤU HÌNH THÔNG SỐ AP VÀ ÁP DỤNG
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    // Không có pass thì để mạng mở
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    // Áp dụng cấu hình
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));    

    // IV. KÍCH HOẠT
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));                   // Đặt chế độ hoạt động là AP
    ESP_ERROR_CHECK(esp_wifi_start());                                  // Phát wifi

    // LOGGING
    ESP_LOGI(TAG_SOFTAP, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}


// WIFI STATION MODE
#define ESP_WIFI_SSID   "Quang Trung"
#define ESP_WIFI_PASS   "0913048530"

static const char *TAG_STA = "wifi STA";
void wifi_init_sta(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_wifi_connect();
    ESP_LOGI(TAG_STA, "wifi_init_sta finished.");
}
