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
// from: "components/pahoMQTT" folder
#include "MQTTAsync.h"

#define ADDRESS     "tcp://test.mosquitto.org:1883"
#define CLIENTID    "ExampleClientPub"
#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hellooo"
#define QOS         1
#define TIMEOUT     10000L

int finished = 0;
static const char* TAG_STATUS = "[MQTT-STATUS]";
static const char* TAG_ACTION = "[MQTT-ACTION]";

void conlost(void* context, char* cause) {
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions con_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    ESP_LOGE(TAG_STATUS, "Connection lost");
    if (cause) {
        printf("Cause: %s\n", cause);
    }

    ESP_LOGI(TAG_ACTION, "Reconnecting...");
    con_opts.keepAliveInterval = 20;
    con_opts.cleansession = 1;

    if ((rc = MQTTAsync_connect(client, &con_opts)) != MQTTASYNC_SUCCESS) {
        ESP_LOGE(TAG_STATUS,"Failed to start connect, return code %d", rc);
        finished = 1;    
    }
}

void onDisconnectFailure(void* context, MQTTAsync_failureData* response) {
    ESP_LOGE(TAG_STATUS, "Disconncet Failed");
    finished = 1;
}

void onDisconnect(void* context, MQTTAsync_successData* response) {
    ESP_LOGI(TAG_STATUS, "Successful disconnection");
    finished = 1;
}

void onSendFailure(void* context, MQTTAsync_failureData* response) {
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_disconnectOptions dics_opts = MQTTAsync_disconnectOptions_initializer;
    int rc;

    ESP_LOGE(TAG_STATUS, "Message send failed token %d error code %d", response->token, response->code);

    ESP_LOGI(TAG_STATUS, "Disconnecting...");
    dics_opts.onSuccess = onDisconnect;
    dics_opts.onFailure = onDisconnectFailure;
    dics_opts.context = client;
    if ((rc = MQTTAsync_disconnect(client, &dics_opts)) != MQTTASYNC_SUCCESS) {
        ESP_LOGE(TAG_STATUS, "Failed to start disconnect, return code %d", rc);
        exit(EXIT_FAILURE);
    }
}

// Something wrong on this
void onSend(void* context, MQTTAsync_successData* response) {
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_disconnectOptions dics_opts = MQTTAsync_disconnectOptions_initializer;
    int rc;

    ESP_LOGI(TAG_STATUS, "Message with token value %d delivery confirmed", response->token);

    dics_opts.onSuccess = onDisconnect;
    dics_opts.onFailure = onDisconnectFailure;
    dics_opts.context = client;
    if ((rc = MQTTAsync_disconnect(client, &dics_opts)) != MQTTASYNC_SUCCESS) {
        ESP_LOGE(TAG_STATUS, "Failed to start disconnect, return code %d", rc);
        exit(EXIT_FAILURE);
    }
}



void app_main(void) {

}






