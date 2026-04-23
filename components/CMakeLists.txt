idf_component_register(SRCS "ESP_WIFI_BLE.c" "ESP_WIFI_BLE.c" "URI_CONFIG.h"
"uri_handler.h" "esp32_peripherals/ESP_BLE.c"
                    INCLUDE_DIRS 
                        "."
                        "esp32_peripherals"
                    REQUIRES    esp_driver_spi 
                                esp_driver_gpio driver 
                                esp_driver_ledc
                                esp_adc
                                esp_driver_dac
                                esp_wifi
                                nvs_flash 
                                bt                              
                                esp_http_server
                                )