// from: C
#include <string.h>
// from: ESP-IDF
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
// from: "annoying_default_funcs" folder
#include "nvs_init_in_main.h"
// from: "components/esp32_peripherals" folder
#include "wifi_sta_ap.h"
#include "URI_CONFIG.h"
// from: "components/coreHTTP" folder
#include "core_http_execute.h"

// static char* TAG = "Main_Example";

void app_main(void)
{
    nvs_flash_init_in_main();

    wifi_init_sta();
    WIFI_WAIT_CONNECT(wifi_event_group);
                                                                
    dns_lookup();
    DNS_WAIT_CONNECT(event_group);
                                                                
    requestHeaderInit();
    xTaskCreate(sendRequest, "tcp_server", 8192, NULL, 5, NULL);
    printf(">>>>Main end<<<<\n");
}




