#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

static const char *TAG = "BLE_WIFI_CFG";

// ─── UUID definitions ─────────────────────────────────────────────────────────
#define SERVICE_UUID        0x1234
#define CHAR_WRITE_UUID     0x5678
#define CHAR_NOTIFY_UUID    0x9ABC

// ─── Advertising flag ─────────────────────────────────────────────────────────
#define ADV_CONFIG_FLAG      (1 << 0)
#define SCAN_RSP_CONFIG_FLAG (1 << 1)
static uint8_t adv_config_done = 0;

// ─── GATTS handles & state ────────────────────────────────────────────────────
static uint16_t service_handle      = 0;
static uint16_t char_write_handle   = 0;
static uint16_t char_notify_handle  = 0;
static uint16_t conn_id             = 0xFFFF;
static esp_gatt_if_t s_gatts_if     = 0;

static bool client_connected = false;

// ─── WiFi credentials ─────────────────────────────────────────────────────────
static char ssid[32]     = {0};
static char password[64] = {0};

// ─── Advertising params ───────────────────────────────────────────────────────
static esp_ble_adv_params_t adv_params = {
    .adv_int_min       = 0x20,
    .adv_int_max       = 0x40,
    .adv_type          = ADV_TYPE_IND,
    .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
    .channel_map       = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// ─── Forward declarations ─────────────────────────────────────────────────────
static void wifi_connect(void);
static void notify_status(const char *msg);
static void parse_data(char *data);

// ─── BLE Notify ───────────────────────────────────────────────────────────────
static void notify_status(const char *msg)
{
    if (!client_connected || conn_id == 0xFFFF || s_gatts_if == 0 || char_notify_handle == 0) {
        ESP_LOGW(TAG, "Cannot notify: client not ready");
        return;
    }

    esp_err_t ret = esp_ble_gatts_send_indicate(
        s_gatts_if,
        conn_id,
        char_notify_handle,
        strlen(msg),
        (uint8_t *)msg,
        false   // false = Notification (không cần ACK); true = Indication (cần ACK)
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Send notify failed: %s", esp_err_to_name(ret));
    }
}

// ─── Parse JSON đơn giản ──────────────────────────────────────────────────────
// Expected format: {"ssid":"your_ssid","password":"your_pass"}
static void parse_data(char *data)
{
    // 1. Tìm vị trí dấu phẩy đầu tiên làm dấu phân tách
    char *comma_ptr = strchr(data, ',');

    if (!comma_ptr) {
        ESP_LOGE(TAG, "Invalid format: missing comma delimiter");
        notify_status("ERR_INVALID_FORMAT");
        return;
    }

    // 2. Tính toán độ dài các phần
    // ssid_len = vị trí dấu phẩy - vị trí bắt đầu
    size_t ssid_len = comma_ptr - data;
    // pass_ptr bắt đầu ngay sau dấu phẩy
    char *pass_ptr = comma_ptr + 1;
    size_t pass_len = strlen(pass_ptr);

    // 3. Validate độ dài trước khi copy (trừ 1 byte cho ký tự kết thúc chuỗi '\0')
    if (ssid_len >= sizeof(ssid) || pass_len >= sizeof(password)) {
        ESP_LOGE(TAG, "Data too long: ssid_len=%zu, pass_len=%zu", ssid_len, pass_len);
        notify_status("ERR_OVERFLOW");
        return;
    }

    // 4. Xóa buffer cũ và copy dữ liệu mới
    memset(ssid, 0, sizeof(ssid));
    memset(password, 0, sizeof(password));

    strncpy(ssid, data, ssid_len); // Chỉ copy đến trước dấu phẩy
    strcpy(password, pass_ptr);    // Copy phần còn lại sau dấu phẩy

    ESP_LOGI(TAG, "SSID: %s", ssid);
    ESP_LOGI(TAG, "PASS: [hidden, len=%zu]", strlen(password));

    wifi_connect();
}

// ─── WiFi connect ─────────────────────────────────────────────────────────────
static void wifi_connect(void)
{
    ESP_LOGI(TAG, "Connecting to WiFi: %s", ssid);

    // Disconnect trước nếu đang kết nối
    esp_wifi_disconnect();

    wifi_config_t cfg = {0};
    strlcpy((char *)cfg.sta.ssid,     ssid,     sizeof(cfg.sta.ssid));
    strlcpy((char *)cfg.sta.password, password, sizeof(cfg.sta.password));

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "wifi_set_config failed: %s", esp_err_to_name(err));
        notify_status("ERR_WIFI_CFG");
        return;
    }

    esp_wifi_connect();
}

// ─── BLE Advertising ──────────────────────────────────────────────────────────
static void ble_start_advertising(void)
{
    esp_ble_gap_set_device_name("ESP32-WiFiConfig");

    // Đặt flag trước khi gọi config (tránh race condition)
    adv_config_done = ADV_CONFIG_FLAG | SCAN_RSP_CONFIG_FLAG;

    // Adv data: chỉ flag + UUID, KHÔNG include_name (tránh duplicate với scan rsp)
    esp_ble_adv_data_t adv_data = {
        .set_scan_rsp        = false,
        .include_name        = false,
        .include_txpower     = true,
        .min_interval        = 0x0006,
        .max_interval        = 0x0010,
        .appearance          = 0x00,
        .manufacturer_len    = 0,
        .p_manufacturer_data = NULL,
        .service_data_len    = 0,
        .p_service_data      = NULL,
        .service_uuid_len    = 0,
        .p_service_uuid      = NULL,
        .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
    };

    // Scan response: tên thiết bị
    esp_ble_adv_data_t scan_rsp_data = {
        .set_scan_rsp        = true,
        .include_name        = true,
        .include_txpower     = false,
        .manufacturer_len    = 0,
        .p_manufacturer_data = NULL,
    };

    esp_ble_gap_config_adv_data(&adv_data);
    esp_ble_gap_config_adv_data(&scan_rsp_data);
}

// ─── GAP callback ─────────────────────────────────────────────────────────────
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~ADV_CONFIG_FLAG);
            if (adv_config_done == 0) {
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;

        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
            if (adv_config_done == 0) {
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;

        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Advertising started successfully");
            } else {
                ESP_LOGE(TAG, "Advertising start failed, status=%d",
                         param->adv_start_cmpl.status);
            }
            break;

        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Advertising stopped");
            }
            break;

        default:
            break;
    }
}

// ─── GATTS callback ───────────────────────────────────────────────────────────
static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param)
{
    switch (event) {

        // 1. App registered → lưu gatts_if → tạo service
        case ESP_GATTS_REG_EVT: {
            s_gatts_if = gatts_if;
            ESP_LOGI(TAG, "GATTS registered, app_id=%d", param->reg.app_id);

            esp_gatt_srvc_id_t service_id = {
                .is_primary       = true,
                .id.inst_id       = 0,
                .id.uuid.len      = ESP_UUID_LEN_16,
                .id.uuid.uuid.uuid16 = SERVICE_UUID,
            };
            // num_handle = 1 service + 2 chars + 2 char_decl + 1 CCCD = 6, dùng 8 cho an toàn
            esp_ble_gatts_create_service(gatts_if, &service_id, 8);
            break;
        }

        // 2. Service created → start service → add WRITE characteristic
        case ESP_GATTS_CREATE_EVT: {
            service_handle = param->create.service_handle;
            ESP_LOGI(TAG, "Service created, handle=%d", service_handle);

            esp_ble_gatts_start_service(service_handle);

            // Add WRITE characteristic
            esp_bt_uuid_t char_write_uuid = {
                .len          = ESP_UUID_LEN_16,
                .uuid.uuid16  = CHAR_WRITE_UUID,
            };
            esp_gatt_char_prop_t write_prop = ESP_GATT_CHAR_PROP_BIT_WRITE
                                            | ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
            esp_ble_gatts_add_char(service_handle,
                                   &char_write_uuid,
                                   ESP_GATT_PERM_WRITE,
                                   write_prop,
                                   NULL, NULL);
            break;
        }

        // 3. Characteristic added → lưu handle → add char tiếp theo hoặc CCCD
        case ESP_GATTS_ADD_CHAR_EVT: {
            uint16_t uuid16 = param->add_char.char_uuid.uuid.uuid16;
            ESP_LOGI(TAG, "Char added uuid=0x%04X handle=%d",
                     uuid16, param->add_char.attr_handle);

            if (uuid16 == CHAR_WRITE_UUID) {
                char_write_handle = param->add_char.attr_handle;

                // Add NOTIFY characteristic
                esp_bt_uuid_t char_notify_uuid = {
                    .len         = ESP_UUID_LEN_16,
                    .uuid.uuid16 = CHAR_NOTIFY_UUID,
                };
                esp_gatt_char_prop_t notify_prop = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
                esp_ble_gatts_add_char(service_handle,
                                       &char_notify_uuid,
                                       ESP_GATT_PERM_READ,
                                       notify_prop,
                                       NULL, NULL);

            } else if (uuid16 == CHAR_NOTIFY_UUID) {
                char_notify_handle = param->add_char.attr_handle;

                // Add CCCD descriptor cho notify characteristic
                // Nếu không có CCCD, client KHÔNG nhận được notification
                esp_bt_uuid_t cccd_uuid = {
                    .len         = ESP_UUID_LEN_16,
                    .uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,  // 0x2902
                };
                uint8_t cccd_val[2]    = {0x00, 0x00};
                esp_attr_value_t cccd  = {
                    .attr_max_len = 2,
                    .attr_len     = 2,
                    .attr_value     = cccd_val,
                };
                esp_ble_gatts_add_char_descr(service_handle,
                                             &cccd_uuid,
                                             ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                             &cccd,
                                             NULL);
            }
            break;
        }

        // 4. Descriptor (CCCD) added
        case ESP_GATTS_ADD_CHAR_DESCR_EVT:
            ESP_LOGI(TAG, "CCCD descriptor added, handle=%d",
                     param->add_char_descr.attr_handle);
            break;

        // 5. Client kết nối → dừng advertising
        case ESP_GATTS_CONNECT_EVT:
            conn_id          = param->connect.conn_id;
            client_connected = true;
            ESP_LOGI(TAG, "Client connected, conn_id=%d", conn_id);
            // Dừng advertising khi đã có client
            esp_ble_gap_stop_advertising();
            break;

        // 6. Client ngắt kết nối → reset state → restart advertising
        case ESP_GATTS_DISCONNECT_EVT:
            client_connected = false;
            conn_id          = 0xFFFF;
            ESP_LOGI(TAG, "Client disconnected, restarting advertising");
            ble_start_advertising();
            break;

        // 7. Client ghi dữ liệu vào WRITE characteristic
        case ESP_GATTS_WRITE_EVT: {
            if (param->write.handle == char_write_handle) {
                // Kiểm tra độ dài an toàn
                uint16_t len = param->write.len;
                if (len >= 200) len = 199;

                char data[200] = {0};
                memcpy(data, param->write.value, len);
                // data[len] = 0; đã được đảm bảo bởi {0} ở trên

                ESP_LOGI(TAG, "Write received [%d bytes]: %s", len, data);
                parse_data(data);
            }

            // Gửi response nếu client yêu cầu (write with response)
            // Không làm bước này → client bị timeout/error
            if (param->write.need_rsp) {
                esp_ble_gatts_send_response(gatts_if,
                                            param->write.conn_id,
                                            param->write.trans_id,
                                            ESP_GATT_OK,
                                            NULL);
            }
            break;
        }

        // 8. Read event (optional, cho debug)
        case ESP_GATTS_READ_EVT:
            ESP_LOGI(TAG, "Read event, handle=%d", param->read.handle);
            break;

        default:
            break;
    }
}

// ─── WiFi event handler ───────────────────────────────────────────────────────
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi STA started");
                break;

            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WiFi connected to AP");
                break;

            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t *disc =
                    (wifi_event_sta_disconnected_t *)event_data;
                ESP_LOGW(TAG, "WiFi disconnected, reason=%d", disc->reason);
                notify_status("WIFI_FAIL");
                break;
            }

            default:
                break;
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            notify_status("WIFI_OK");
        }
    }
}

// ─── WiFi init ────────────────────────────────────────────────────────────────
static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Đăng ký cả WIFI_EVENT và IP_EVENT
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

// ─── BLE init ─────────────────────────────────────────────────────────────────
static void ble_init(void)
{
    // Giải phóng bộ nhớ BT Classic (không dùng)
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(0));

    // Set MTU lớn hơn để hỗ trợ gói dữ liệu WiFi credentials dài
    ESP_ERROR_CHECK(esp_ble_gatt_set_local_mtu(200));

    ble_start_advertising();
}

// ─── Entry point ──────────────────────────────────────────────────────────────
void app_main(void)
{
    // Init NVS (bắt buộc cho WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Initializing WiFi...");
    wifi_init();

    ESP_LOGI(TAG, "Initializing BLE...");
    ble_init();

    ESP_LOGI(TAG, "BLE WiFi Config ready. Waiting for client...");
}