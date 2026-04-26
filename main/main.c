#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
// #include "esp_http_server.h"
// #include "URI_CONFIG.h"
#include "wifi_sta_ap.h"



/* Hàm khởi tạo Server */
// httpd_handle_t start_webserver() {
//     httpd_handle_t server = NULL;
//     httpd_config_t config = HTTPD_DEFAULT_CONFIG();

//     // Start server
//     if (httpd_start(&server, &config) == ESP_OK) {
//         // Dang ky handler
//         httpd_register_uri_handler(server, &uri_info);
//         httpd_register_uri_handler(server, &uri_led);
//         httpd_register_uri_handler(server, &uri_on);
//         httpd_register_uri_handler(server, &uri_off);

//         return server;
//     }

//     return NULL;
// }



#define ESP_WIFI_SSID   "Quang Trung"
#define ESP_WIFI_PASS   "0913048530"

// void wifi_init_sta(void) {
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());
//     esp_netif_create_default_wifi_sta();

//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_wifi_init(&cfg));

//     wifi_config_t wifi_config = {
//         .sta = {
//             .ssid = ESP_WIFI_SSID,
//             .password = ESP_WIFI_PASS,
//             .threshold.authmode = WIFI_AUTH_WPA2_PSK
//         }
//     };

//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
//     ESP_ERROR_CHECK(esp_wifi_start());

//     esp_wifi_connect();
//     ESP_LOGI(TAG, "wifi_init_sta finished.");
// }

#define PORT 3333
static void tcp_server_task(void *pvParameters) {
    char rx_buffer[128];
    char addr_str[32];
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    // Tao socket
    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    listen(listen_sock, 1);

    ESP_LOGI(TAG, "Server listening on port %d", PORT);

    while (1) {
        struct sockaddr_in source_addr;
        socklen_t addr_len = sizeof(source_addr);

        // Chap nhan ket noi
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) { 
            continue; 
        }

        inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        ESP_LOGI(TAG, "Socket accepted ip: %s", addr_str);
        uint8_t led_state = 0;

        while (1) {
            // Nhan du lieu
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len <= 0) {
                break;
            }

            rx_buffer[len] = 0; // NULL terminal
            ESP_LOGI(TAG, "Received: %s", rx_buffer);
            
            // Gui phan hoi
            send(sock, rx_buffer, len, 0);
            
            if (strcmp(rx_buffer, "On led") == 0) {
                led_state = 1;
                gpio_set_level(LED_PIN, led_state);
            }
            else if (strcmp(rx_buffer, "Off led") == 0) {
                led_state = 0;
                gpio_set_level(LED_PIN, led_state);                
            }           
        }
            close(sock);
    }
    vTaskDelete(NULL);
}


#define PORT_UDP 4023
static const char *TAG2 = "UDP_SERVER";

static void udp_server_task(void *pvParameters) {
    char rx_buffer[128];
    char addr_str[32];
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT_UDP);

    while (1) {
        // 1. Tạo socket UDP (SOCK_DGRAM)
        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG2, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG2, "Socket created");

        // 2. Bind socket vào Port
        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG2, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG2, "Server listening on port %d", PORT_UDP);

        while (1) {
            struct sockaddr_storage source_addr; // Cấu trúc chứa thông tin người gửi
            socklen_t socklen = sizeof(source_addr);

            // 3. Nhận dữ liệu (Dùng recvfrom)
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            if (len < 0) {
                ESP_LOGE(TAG2, "recvfrom failed: errno %d", errno);
                break;
            } else {
                rx_buffer[len] = 0; // Null terminate
                
                // Lấy IP người gửi để in log
                inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
                ESP_LOGI(TAG2, "Received %d bytes from %s: %s", len, addr_str, rx_buffer);

                // 4. Xử lý logic LED
                if (strcmp(rx_buffer, "On led") == 0) {
                    gpio_set_level(LED_PIN, 1);
                } else if (strcmp(rx_buffer, "Off led") == 0) {
                    gpio_set_level(LED_PIN, 0);
                }

                // 5. Gửi phản hồi (Dùng sendto)
                int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                if (err < 0) {
                    ESP_LOGE(TAG2, "Error occurred during sending: errno %d", errno);
                    break;
                }
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG2, "Shutting down socket and reconnecting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

// void dns_lookup_task(void *pvParameters) {

//     vTaskDelay(pdMS_TO_TICKS(2000));

//     const struct addrinfo hints = {
//         .ai_family = AF_INET,
//         .ai_socktype = SOCK_STREAM
//     };

//     struct addrinfo *res;

//     ESP_LOGI(TAG, "Dang truy van DNS...");

//     int err = getaddrinfo("facebook.com", "443", &hints, &res);

//     if (err != 0 || res == NULL) {
//         ESP_LOGE(TAG, "DNS lookup failed err = %d res = %p", err, res);
//         return;
//     }else {
   
//         // Lay dia chi IP dau tien
//         struct in_addr *addr;
//         addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
//         ESP_LOGI(TAG, "DNS lookup succeeded, IP = %s", inet_ntoa(*addr));
        
//         freeaddrinfo(res);
//     }
//     vTaskDelete(NULL);
// }


void app_main(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
    // wifi_init_sta();

    // xTaskCreate(dns_lookup_task, "dns_lookup", 4096, NULL, 5, NULL);
    xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);
    xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 5, NULL);

    // ESP_LOGI(TAG, "Starting Web Server");
    // start_webserver();   
}

