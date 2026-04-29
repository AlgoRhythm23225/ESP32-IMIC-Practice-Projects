// from: IDF-ESP
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
// from: "annoying_default_funcs" folder
#include "nvs_init_in_main.h"
// from: "components/esp32_peripherals" folder
#include "wifi_sta_ap.h"
#include "esp_client_http.h"
#include "core_http_client.h"

// static char* TAG = "Main_Example";
void app_main(void)
{
    nvs_flash_init_in_main();
    wifi_init_sta();

    WIFI_WAIT_CONNECT(wifi_event_group);

    http_state_t http_state = GET;
    xTaskCreate(&http_test_task, "http_test_task", 8192, &http_state, 5, NULL);
}




