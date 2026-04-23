#include <string.h>

#include "esp_log.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"


extern const char *TAG;

// ─── UUID definitions ─────────────────────────────────────────────────────────
#define SERVICE_UUID        0x1234
#define CHAR_WRITE_UUID     0x5678
#define CHAR_NOTIFY_UUID    0x9ABC

// ─── Advertising flag ─────────────────────────────────────────────────────────
#define ADV_CONFIG_FLAG      (1 << 0)
#define SCAN_RSP_CONFIG_FLAG (1 << 1)
extern uint8_t adv_config_done;

// ─── GATTS handles & state ────────────────────────────────────────────────────
extern uint16_t service_handle;
extern uint16_t char_write_handle;
extern uint16_t char_notify_handle;
extern uint16_t conn_id;
extern esp_gatt_if_t s_gatts_if;

extern bool client_connected;

// ─── WiFi credentials ─────────────────────────────────────────────────────────
extern char ssid[32];
extern char password[64];

extern esp_ble_adv_params_t adv_params;

// ─── Parse JSON đơn giản ──────────────────────────────────────────────────────
// Expected format: {"ssid":"your_ssid","password":"your_pass"}
void parse_data(char *data);

// ─── BLE init ─────────────────────────────────────────────────────────────────
void ble_init(void);

// ─── BLE Advertising ──────────────────────────────────────────────────────────
void ble_start_advertising(void);

// ─── BLE Notify ───────────────────────────────────────────────────────────────
void notify_status(const char *msg);

// ─── GATTS callback ───────────────────────────────────────────────────────────
void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param);

// ─── GAP callback ─────────────────────────────────────────────────────────────
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

// Cách Khai báo trong main
// #include <stdio.h>
// #include <stdlib.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"

// #include "esp_netif.h"
// #include "esp_event.h"
// #include "nvs_flash.h"

// #include "ESP_BLE.h"
// #include "ESP_WIFI_BLE.h"

// void app_main(void)
// {
//     // Init NVS (bắt buộc cho WiFi)
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     ESP_LOGI(TAG, "Initializing WiFi.................................................");
//     wifi_init();

//     ESP_LOGI(TAG, "Initializing BLE..................................................");
//     ble_init();

//     ESP_LOGI(TAG, "BLE WiFi Config ready. Waiting for client...");
// }