idf_component_register(SRCS "ESP_WIFI_BLE.c" "ESP_BLE.c" "ESP_DAC.c" "ESP_ADC.c" "ESP_SPI.c" "rc522.c" "rfid.c" "ESP_BLE.c" "ESP_WIFI_BLE.c"
                    INCLUDE_DIRS "."
                    REQUIRES    esp_driver_spi 
                                esp_driver_gpio driver 
                                esp_driver_ledc
                                esp_adc
                                esp_driver_dac
                                esp_wifi
                                nvs_flash 
                                bt                              
                                )