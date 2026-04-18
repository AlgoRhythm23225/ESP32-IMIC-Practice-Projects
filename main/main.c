#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "ESP_BLE.h"
#include "ESP_WIFI_BLE.h"

void app_main(void)
{
    // Init NVS (bắt buộc cho WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Initializing WiFi.................................................");
    wifi_init();

    ESP_LOGI(TAG, "Initializing BLE..................................................");
    ble_init();

    ESP_LOGI(TAG, "BLE WiFi Config ready. Waiting for client...");
}