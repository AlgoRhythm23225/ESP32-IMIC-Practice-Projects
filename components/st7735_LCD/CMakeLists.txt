idf_component_register(SRCS "st7735.c" "Example/st7735_example.c" 
                    INCLUDE_DIRS 
                        "Include" "Example/Include"
                    REQUIRES
                        esp_driver_spi 
                        esp_driver_gpio driver )
