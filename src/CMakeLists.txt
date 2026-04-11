idf_component_register(SRCS "ESP_SPI.c" "rc522.c" "rfid.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_spi esp_driver_gpio driver esp_driver_ledc)