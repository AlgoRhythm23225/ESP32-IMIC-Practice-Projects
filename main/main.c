// from: C
#include <string.h>
// from: ESP-IDF
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "driver/uart.h"
// from: "annoying_default_funcs" folder
#include "nvs_init_in_main.h"
// from: "components/esp32_peripherals" folder
#include "wifi_sta_ap.h"
#include "tcp_udp_socket.h"
// from: "components/coreHTTP" folder
#include "core_http_client.h"
#include "URI_CONFIG.h"

// static char* TAG = "Main_Example";
void uartTransmitData(char* trans_data);

HTTPRequestHeaders_t requestHeaders = {0};
uint8_t requestHeaderBubffer[256] = {0};
void requestHeaderInit() {
    HTTPRequestInfo_t requestInfo = {0};

    requestHeaders.pBuffer = requestHeaderBubffer;
    requestHeaders.bufferLen = 256;

    requestInfo.pMethod = HTTP_METHOD_GET;
    requestInfo.methodLen = sizeof(HTTP_METHOD_GET) - 1U;
    // requestInfo.pPath = "/";
    // requestInfo.pathLen = sizeof("/") - 1U;
    requestInfo.pHost = "httpforever.com";
    requestInfo.hostLen = sizeof("httpforever.com") - 1U;
    // requestInfo.reqFlags |= HTTP_REQUEST_KEEP_ALIVE_FLAG;
    

    HTTPStatus_t httpLibraryStatus = HTTPClient_InitializeRequestHeaders(&requestHeaders, &requestInfo);
    if(httpLibraryStatus == HTTPSuccess) {
        ESP_LOGI("[HTTP_INIT]", "SUCCESS================");
    }

    printf("Request header:\n");
    printf("%s\n", requestHeaders.pBuffer);
}

TransportInterface_t transportInterface = {0};
HTTPResponse_t pResponse = {0};

int32_t myPlatformTransportReceive(NetworkContext_t * pNetworkContext,
                                       void * pBuffer,
                                       size_t bytesToRecv) 
{
    return recv(pNetworkContext->socket, pBuffer, bytesToRecv, 0);
}

int32_t myPlatformTransportSend(NetworkContext_t * pNetworkContext,
                                       const void * pBuffer,
                                       size_t bytesToSend) 
{
    return send(pNetworkContext->socket, pBuffer, bytesToSend, 0);
}
extern struct in_addr addr;
void sendRequest(void *pvParameters) {
    HTTPStatus_t httpLibraryStatus = HTTPSuccess;
    uint8_t* requestBody = NULL;
    pResponse.pBuffer = (uint8_t*)malloc(8192);
    pResponse.bufferLen = 8192;

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(sock) {
        ESP_LOGI("[Socket]", "Socket created succesfully, ID=%d===============",sock);
    }
    struct sockaddr_in dest_addr;
    // dest_addr.sin_addr.s_addr = inet_addr("146.190.62.39");
    dest_addr.sin_addr.s_addr = inet_addr(inet_ntoa(addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(80);

    printf("Connecting...\n");
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if(err == 0) {
        ESP_LOGI("[Socket]", "Socket connect succesfully, ID=%d===============",err);
    } else {
        ESP_LOGE("[Socket]", "Socket connect failed, errno=%d===============err=%d",errno,err);
    }
    printf("Connected!\n");
    NetworkContext_t ctx;
    ctx.socket = sock;

    transportInterface.recv = myPlatformTransportReceive;
    transportInterface.send = myPlatformTransportSend;
    transportInterface.pNetworkContext  = &ctx;
    printf("Start send...\n");
    httpLibraryStatus = HTTPClient_Send(&transportInterface,
                                        &requestHeaders,
                                        requestBody,
                                        0,
                                        &pResponse,
                                        0);

    if (httpLibraryStatus == HTTPSuccess) {
        ESP_LOGI("[STATUS]","HTTPClient_Send - success, id=%d", httpLibraryStatus);
    } else {
        ESP_LOGE("[STATUS]","HTTPClient_Send - Something wrong, id=%d", httpLibraryStatus);
    }
    printf("Sent!\n");

    const char* pDateloc = NULL;
    size_t dateLen = 0;
    printf("Reading...\n");
    httpLibraryStatus = HTTPClient_ReadHeader(&pResponse, 
                                            "Date",
                                            sizeof("Date") - 1U,
                                            &pDateloc,
                                            &dateLen );
                           
    if (httpLibraryStatus == HTTPSuccess) {
        ESP_LOGI("[STATUS]","HTTPClient_ReadHeader - success, id=%d", httpLibraryStatus);
    } else {
        ESP_LOGE("[STATUS]","HTTPClient_ReadHeader - Something wrong, id=%d", httpLibraryStatus);
    }

    printf("Read done!\n");
    // printf("%s", pResponse.pBuffer);
    printf("Size of data: %d\n", strlen((char *)pResponse.pBuffer));
    uartTransmitData((char *)pResponse.pBody);

    close(sock);

    vTaskDelete(NULL);
}

#define TX_PIN 17
#define RX_PIN 16
void uart_init() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_NUM_1, &uart_config);

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    uart_driver_install(UART_NUM_1, 1024, 0, 0, NULL, 0);
}
    
void uartTransmitData(char* trans_data) {
    // uart_write_bytes(UART_NUM_1, (const char*)trans_data, strlen(trans_data));
    uart_write_bytes(UART_NUM_1, (const char*)trans_data, 6109);
}


void app_main(void)
{
    nvs_flash_init_in_main();
    wifi_init_sta();
    uart_init();

    requestHeaderInit();

    WIFI_WAIT_CONNECT(wifi_event_group);
    
    xTaskCreate(dns_lookup_task, "look_up_dns", 4096, NULL, 5, NULL);
    DNS_WAIT_CONNECT(event_group);

    xTaskCreate(sendRequest, "tcp_server", 8192, NULL, 5, NULL);
    printf(">>>>Main end<<<<\n");

}




