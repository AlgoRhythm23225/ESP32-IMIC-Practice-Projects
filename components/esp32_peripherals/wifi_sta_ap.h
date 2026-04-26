#include "esp_mac.h"
#include "esp_wifi.h"
#include "driver/gpio.h"

#include "esp_event.h"
#include "esp_log.h"
#include <string.h>

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#define LED_PIN 2


#define EXAMPLE_ESP_WIFI_SSID      "ESP32_AP"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data);
void wifi_init_softap();

void wifi_init_sta();