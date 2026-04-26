idf_component_register(SRCS "rc522.c" "rfid.c"
                    INCLUDE_DIRS "." 
                    REQUIRES    esp_driver_spi 
                                esp_driver_gpio driver 
                                esp_driver_ledc
                                esp_adc
                                esp_driver_dac
                                esp_wifi
                                nvs_flash 
                                bt                              
                                esp_http_server
                                esp32_peripherals)
