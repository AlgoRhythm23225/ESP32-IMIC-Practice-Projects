#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

static const char *TAG = "BLE_WIFI_CFG";

#define SERVICE_UUID        0x1234  
#define CHAR_WRITE_UUID     0x5678
#define CHAR_NOTIFY_UUID    0x9ABC

static uint16_t service_handle;
static uint16_t char_write_handle;
static uint16_t char_notify_handle;
static uint16_t conn_id;
static esp_gatt_if_t s_gatts_if = 0;  // lưu gatts_if thực

static char ssid[32];
static char password[64];

// ─── BLE Advertising ─────────────────────────────────────────────────────────
static void ble_start_advertising(void)
{
    esp_ble_gap_set_device_name("ESP32-WiFiConfig");

    esp_ble_adv_data_t adv_data = {
        .set_scan_rsp        = false,
        .include_name        = true,
        .include_txpower     = false,
        .min_interval        = 0x0006,
        .max_interval        = 0x0010,
        .appearance          = 0x80,
        .manufacturer_len    = 0,
        .p_manufacturer_data = NULL,
        .service_data_len    = 0,
        .p_service_data      = NULL,
        .service_uuid_len    = 0,
        .p_service_uuid      = NULL,
        .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
    };

    esp_ble_adv_data_t scan_rsp_data = {
        .set_scan_rsp = true,
        .include_name = true,
    };

    esp_ble_gap_config_adv_data(&adv_data);
    esp_ble_gap_config_adv_data(&scan_rsp_data);
}

// ─── WiFi connect ────────────────────────────────────────────────────────────
static void wifi_connect(void) {
    ESP_LOGI(TAG, "Connecting WiFi: %s", ssid);

    wifi_config_t cfg = {0};
    strcpy((char*)cfg.sta.ssid, ssid);
    strcpy((char*)cfg.sta.password, password);

    esp_wifi_set_config(WIFI_IF_STA, &cfg);
    esp_wifi_connect();
}

// ─── BLE Notify ──────────────────────────────────────────────────────────────
static void notify_status(const char* msg) {
    esp_ble_gatts_send_indicate(
        s_gatts_if,
        conn_id,
        char_notify_handle,
        strlen(msg),
        (uint8_t*)msg,
        false
    );
}

// ─── Parse JSON đơn giản ─────────────────────────────────────────────────────
static void parse_data(char *data) {
    char *ssid_ptr = strstr(data, "ssid");
    char *pass_ptr = strstr(data, "password");

    if (!ssid_ptr || !pass_ptr) {
        ESP_LOGE(TAG, "Invalid data");
        return;
    }

    sscanf(ssid_ptr, "ssid\":\"%31[^\"]", ssid);
    sscanf(pass_ptr, "password\":\"%63[^\"]", password);  // fix typo "passowrd"

    ESP_LOGI(TAG, "SSID: %s", ssid);
    ESP_LOGI(TAG, "PASS: %s", password);

    wifi_connect();
}

// ─── GAP callback (phải đứng TRƯỚC ble_init) ─────────────────────────────────
static esp_ble_adv_params_t adv_params = {
    .adv_int_min       = 0x20,
    .adv_int_max       = 0x40,
    .adv_type          = ADV_TYPE_IND,
    .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
    .channel_map       = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            esp_ble_gap_start_advertising(&adv_params);
            break;

        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Advertising started");
            } else {
                ESP_LOGE(TAG, "Advertising start failed");
            }
            break;

        default:
            break;
    }
}

// ─── GATTS callback (phải đứng TRƯỚC ble_init) ───────────────────────────────
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
        case ESP_GATTS_REG_EVT:
            s_gatts_if = gatts_if;  // lưu lại để dùng cho notify
            break;

        case ESP_GATTS_CONNECT_EVT:
            conn_id = param->connect.conn_id;
            ESP_LOGI(TAG, "Client connected");
            break;

        case ESP_GATTS_WRITE_EVT:
            if (param->write.handle == char_write_handle) {
                char data[200] = {0};
                memcpy(data, param->write.value, param->write.len);
                ESP_LOGI(TAG, "Received: %s", data);
                parse_data(data);
            }
            break;

        case ESP_GATTS_CREATE_EVT:
            service_handle = param->create.service_handle;
            esp_ble_gatts_start_service(service_handle);
            break;

        default:
            break;
    }
}



// ─── WiFi init ───────────────────────────────────────────────────────────────
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_STA_START)       ESP_LOGI(TAG, "WiFi started");
    if (event_id == WIFI_EVENT_STA_CONNECTED)  { ESP_LOGI(TAG, "WiFi connected"); notify_status("OK"); }
    if (event_id == WIFI_EVENT_STA_DISCONNECTED){ ESP_LOGI(TAG, "WiFi failed");   notify_status("FAIL"); }
}

static void wifi_init(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
}

// ─── BLE init ────────────────────────────────────────────────────────────────
static void ble_init(void)
{
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);

    esp_bluedroid_init();
    esp_bluedroid_enable();

    esp_ble_gap_register_callback(gap_event_handler);
    esp_ble_gatts_register_callback(gatts_event_handler);
    esp_ble_gatts_app_register(0);

    ble_start_advertising();
}

// ─── Entry point ─────────────────────────────────────────────────────────────
void app_main(void)
{
    nvs_flash_init();
    wifi_init();
    ble_init();
    ESP_LOGI(TAG, "BLE WiFi Config ready");
}