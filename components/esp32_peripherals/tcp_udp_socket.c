#include "tcp_udp_socket.h"

// TCP socket
static const char* TAG_TCP_SERVER = "TCP_SERVER";
void tcp_server_task(void *pvParameters) {
    char rx_buffer[128];
    char addr_str[32];
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT_TCP);

    // Tao socket
    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    listen(listen_sock, 1);

    ESP_LOGI(TAG_TCP_SERVER, "Server listening on port %d", PORT_TCP);
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);

    while (1) {

        // Chap nhan ket noi
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) { 
            continue; 
        }   

        inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        ESP_LOGI(TAG_TCP_SERVER, "Socket accepted ip: %s", addr_str);

        while (1) {
            // Nhan du lieu
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len <= 0) {
                break;
            }

            rx_buffer[len] = 0; // NULL terminal
            ESP_LOGI(TAG_TCP_SERVER, "Received: %s", rx_buffer);
            
            // Gui phan hoi
            send(sock, rx_buffer, len, 0);
        }
            close(sock);
    }
    vTaskDelete(NULL);
}

// UDP socket
static const char *TAG_UDP_SERVER = "UDP_SERVER";
void udp_server_task(void *pvParameters) {
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
            ESP_LOGE(TAG_UDP_SERVER, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG_UDP_SERVER, "Socket created");

        // 2. Bind socket vào Port
        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG_UDP_SERVER, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG_UDP_SERVER, "Server listening on port %d", PORT_UDP);

        while (1) {
            struct sockaddr_storage source_addr; // Cấu trúc chứa thông tin người gửi
            socklen_t socklen = sizeof(source_addr);

            // 3. Nhận dữ liệu (Dùng recvfrom)
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            if (len < 0) {
                ESP_LOGE(TAG_UDP_SERVER, "recvfrom failed: errno %d", errno);
                break;
            } else {
                rx_buffer[len] = 0; // Null terminate
                
                // Lấy IP người gửi để in log
                inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
                ESP_LOGI(TAG_UDP_SERVER, "Received %d bytes from %s: %s", len, addr_str, rx_buffer);

                // 4. Xử lý logic LED
                if (strcmp(rx_buffer, "On led") == 0) {
                    gpio_set_level(LED_PIN, 1);
                } else if (strcmp(rx_buffer, "Off led") == 0) {
                    gpio_set_level(LED_PIN, 0);
                }

                // 5. Gửi phản hồi (Dùng sendto)
                int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                if (err < 0) {
                    ESP_LOGE(TAG_UDP_SERVER, "Error occurred during sending: errno %d", errno);
                    break;
                }
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG_UDP_SERVER, "Shutting down socket and reconnecting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}