#include "esp_http_server.h"
#define LED_PIN 2

/* Hàm xử lý khi người dùng truy cập trang chủ */
esp_err_t info_handler(httpd_req_t *req) {
    const char* resp_str = "<h1> Xin chao! day la ESP32 Server</h1><p>Chao mung toi server dau tien</p>";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

// Led
esp_err_t led_handler(httpd_req_t *req) {
    const char* resp_str = 
        "<html>"
        "<head><style>"
        ".button { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; "
        "text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer;}"
        ".off { background-color: #f44336; }"
        "</style></head>"
        "<body>"
        "<h1>ESP32 Control Panel</h1>"
        "<a href=\"/led_on\" class=\"button\">BAT LED</a>"
        "<a href=\"/led_off\" class=\"button off\">TAT LED</a>"
        "</body>"
        "</html>";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

// On_led
esp_err_t led_on_handler(httpd_req_t *req) {
    gpio_set_level(LED_PIN, 1);
    const char* resp =
        "<html>"

        "<head><style>"
        ".button { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; "
        "text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer;}"
        ".off { background-color: #f44336; }"
        "</style></head>"

        "<body>"
        "<h1>ESP32 Control Panel</h1>"
        "<a href=\"/led_off\" class=\"button off\">TAT LED</a>"
        "</body>"

        "</html>";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

// OFF_led
esp_err_t led_off_handler(httpd_req_t *req) {
    gpio_set_level(LED_PIN, 0);
    const char* resp = 
        "<html>"

        "<head><style>"
        ".button { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; "
        "text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer;}"
        ".off { background-color: #f44336; }"
        "</style></head>"

        "<body>"
        "<h1>ESP32 Control Panel</h1>"
        "<a href=\"/led_on\" class=\"button\">BAT LED</a>"
        "</body>"
        
        "</html>";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}