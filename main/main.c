// from: C
#include <string.h>
// from: ESP-IDF
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
// from: "annoying_default_funcs" folder
#include "nvs_init_in_main.h"
// project

void app_main(void) {
    nvs_flash_init_in_main();
    // wifi_init_sta();
    // WIFI_WAIT_CONNECT(wifi_event_group);

    // mqtt_app_start();
    // xTaskCreate(weather_task, "weather_task", 8192, NULL, 5, NULL);
    
    
}


