idf_component_register(SRCS 
                        # In Forder
                        "esp_client_http.c" "ESP_MQTT.c" "tcp_udp_socket.c" "ESP_ADC.c" "ESP_DAC.c" "ESP_SPI.c" "ESP_BLE.c"
                        "wifi_sta_ap.c" "ESP_WIFI_BLE.c" "ESP_MQTT.c" 
                        # Outside Folder
                        "../../managed_components/espressif__cjson/cJSON/cJSON.c"
                    INCLUDE_DIRS "Include"
                                "../../managed_components/espressif__cjson/cJSON"
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
                                mqtt
                    EMBED_TXTFILES
                                "../../main/AWS_IOT_Cert/AmazonRootCA1.pem"
                                "../../main/AWS_IOT_Cert/Device_Certificate.crt"
                                "../../main/AWS_IOT_Cert/Private_Key.key"
                                )
