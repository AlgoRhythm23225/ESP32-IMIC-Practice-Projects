#include "uri_handler.h"

static const char* TAG = "[Test DNS]";

/* Cấu hình URI */
httpd_uri_t uri_info = {
    .uri        = "/info",
    .method     = HTTP_GET,
    .handler    = info_handler,
    .user_ctx   = NULL
};
httpd_uri_t uri_led = {
    .uri        = "/led",
    .method     = HTTP_GET,
    .handler    = led_handler,
    .user_ctx   = NULL
};

httpd_uri_t uri_on   = { 
    .uri = "/led_on", 
    .method = HTTP_GET, 
    .handler = led_on_handler 
};

httpd_uri_t uri_off  = { 
    .uri = "/led_off", 
    .method = HTTP_GET, 
    .handler = led_off_handler 
};

/* Hàm khởi tạo Server */
httpd_handle_t start_webserver() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start server
    if (httpd_start(&server, &config) == ESP_OK) {
        // Dang ky handler
        httpd_register_uri_handler(server, &uri_info);
        httpd_register_uri_handler(server, &uri_led);
        httpd_register_uri_handler(server, &uri_on);
        httpd_register_uri_handler(server, &uri_off);

        return server;
    }

    return NULL;
}

#define DNS_DONE_BIT (1 << 1)
#define DNS_WAIT_CONNECT(x)     xEventGroupWaitBits((x), DNS_DONE_BIT, pdFALSE, pdTRUE, portMAX_DELAY)
EventGroupHandle_t event_group;
struct in_addr addr;
void dns_lookup() {
    event_group = xEventGroupCreate();

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };

    struct addrinfo *res;

    ESP_LOGI(TAG, "Dang truy van DNS...");

    int err = getaddrinfo("httpforever.com", "80", &hints, &res);
    
    if (err != 0 || res == NULL) {
        ESP_LOGE(TAG, "DNS lookup failed err = %d res = %p", err, res);
        return;
    }else {   
        // Lay dia chi IP dau tien
        addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
        xEventGroupSetBits(event_group, DNS_DONE_BIT);
        ESP_LOGI(TAG, "DNS lookup succeeded, IP = %s", inet_ntoa(addr));
        freeaddrinfo(res);
    }
}

