#include "esp_http_server.h"
#include "uri_handler.h"

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

httpd_uri_t uri_on   = { .uri = "/led_on", .method = HTTP_GET, .handler = led_on_handler };
httpd_uri_t uri_off  = { .uri = "/led_off", .method = HTTP_GET, .handler = led_off_handler };