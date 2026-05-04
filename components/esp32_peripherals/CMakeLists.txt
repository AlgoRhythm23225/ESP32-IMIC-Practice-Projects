idf_component_register(SRCS "tcp_udp_socket.c" "ESP_ADC.c" "ESP_DAC.c" "ESP_SPI.c" "ESP_BLE.c"
                            "wifi_sta_ap.c" "ESP_WIFI_BLE.c" "tcp_udp_socket.c" 
                    INCLUDE_DIRS "Include"
                    REQUIRES    esp_driver_spi 
                                esp_driver_gpio driver 
                                esp_driver_ledc
                                esp_adc
                                esp_driver_dac
                                esp_wifi
                                nvs_flash 
                                bt                              
                                esp_http_server
                                esp_http_client
                                esp-tls
                                )
