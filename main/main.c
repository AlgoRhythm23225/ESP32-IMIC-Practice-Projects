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
#include "driver/gpio.h"

#define LED_PIN_1 GPIO_NUM_25
#define LED_PIN_2 GPIO_NUM_26

void led_init() {
    gpio_reset_pin(LED_PIN_1);
    gpio_reset_pin(LED_PIN_2);

    gpio_set_direction(LED_PIN_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PIN_2, GPIO_MODE_OUTPUT);
}

void blink_led_1(void *vParameter) {
    while (1) {
        gpio_set_level(LED_PIN_1, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_PIN_1, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void blink_led_2(void *vParameter) {
    while (1) {
        gpio_set_level(LED_PIN_2, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_set_level(LED_PIN_2, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    nvs_flash_init_in_main();

    led_init();
    vTaskDelay(pdMS_TO_TICKS(100));

    xTaskCreate(blink_led_1, "blink led 1", 1024, NULL, 4, NULL);
    xTaskCreate(blink_led_2, "blink led 2", 1024, NULL, 4, NULL);
    // wifi_init_sta();
    // WIFI_WAIT_CONNECT(wifi_event_group);

    // mqtt_app_start();
    // xTaskCreate(weather_task, "weather_task", 8192, NULL, 5, NULL);
    
    
}


