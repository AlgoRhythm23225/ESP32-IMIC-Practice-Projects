// from: C
#include <string.h>
// from: ESP-IDF
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
// from: "annoying_default_funcs" folder
#include "nvs_init_in_main.h"
// project

QueueHandle_t xUartEventQueue;
QueueHandle_t xLogicQueue;

typedef struct {
    uint8_t payLoad[128];
    uint16_t length;
} data_packet_t;

#define TX_PIN (GPIO_NUM_13)
#define RX_PIN (GPIO_NUM_35)
#define UART_PORT_NUM (UART_NUM_1)
#define BUF_SIZE (1024)

void init_uart_with_queue(void) {
    uart_config_t uart_cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_cfg));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &xUartEventQueue, 0));
}

void uart_receiver_task(void *pvParameters){
    uart_event_t event;
    uint8_t *dtmp = (uint8_t*)malloc(BUF_SIZE);
    while (1) {
        if (xQueueReceive(xUartEventQueue, (void*)&event, portMAX_DELAY)) {
            ESP_LOGI("[Queue]", "Got string from UART");
            memset(dtmp, 0, BUF_SIZE);
        }

        switch (event.type) {
            // When uart received data
            case UART_DATA:
                if (event.size > 0) {
                    data_packet_t packet;
                    int len = uart_read_bytes(UART_PORT_NUM, dtmp, event.size, portMAX_DELAY);

                    if (len < sizeof(packet.payLoad)) {
                        memcpy(packet.payLoad, dtmp, len);
                        packet.length = len;
                    
                        if (xQueueSend(xLogicQueue, &packet, 0) != pdTRUE) {
                            ESP_LOGW("[Queue]", "The queue is full! Some data may lost!");
                        }
                    }
                }
                break;
            
            // FIX Error
            case UART_FIFO_OVF: 
                ESP_LOGW("[UART]", "UART FIFO overflow!");
                uart_flush_input(UART_PORT_NUM);
                xQueueReset(xUartEventQueue);
                break;

            case UART_BUFFER_FULL:
                ESP_LOGW("[UART]", "Ring buffer full!");
                uart_flush_input(UART_PORT_NUM);
                xQueueReset(xUartEventQueue);
                break;

            default:
                break;
        }
    }   
    free(dtmp);
    vTaskDelete(NULL);
}

void logic_process_task(void *vParameter){
    data_packet_t received_packet;
    while (1) {
        if (xQueueReceive(xLogicQueue, &received_packet, portMAX_DELAY)) {
            ESP_LOGI("[Queue]", "Got the string %d characters from Task 1", received_packet.length);
            printf("String: %.*s\n", received_packet.length, received_packet.payLoad);
        }
    }
    vTaskDelete(NULL);
}

#define BUFFER_SIZE 5
#define TOTAL_ITEM  15

int ring_buffer[BUF_SIZE];
int head = 0;
int tail = 0;

int total_consumed_count = 0;

SemaphoreHandle_t xBufMutex = NULL;
SemaphoreHandle_t xEmptySlotsSem = NULL;
SemaphoreHandle_t xFilledSlotsSem = NULL;

void producer_task(void *pvParameters) {
    int id = (int)pvParameters;

    for (int i = 0; i < 3; i++) {
        xSemaphoreTake(xEmptySlotsSem, portMAX_DELAY);

        xSemaphoreTake(xBufMutex, portMAX_DELAY);

        ring_buffer[head] = id;
        head = (head + 1) % BUFFER_SIZE;
        printf("Producer %d: Ghi thanh cong gia tri %d\n", id, id);

        xSemaphoreGive(xBufMutex);

        xSemaphoreGive(xFilledSlotsSem);

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    printf("--> Producer %d: Da hoan thanh cong viec\n", id);
    vTaskDelete(NULL);
}

void consumer_task(void *pvParameter) {
    int consumer_id = (int)pvParameter;

    while(1) {
        xSemaphoreTake(xBufMutex, portMAX_DELAY);
        if (total_consumed_count >= TOTAL_ITEM) {
            xSemaphoreGive(xBufMutex);
            break;
        }
        xSemaphoreGive(xBufMutex);

        xSemaphoreTake(xFilledSlotsSem, portMAX_DELAY);
        
        xSemaphoreTake(xBufMutex, portMAX_DELAY);

        if (total_consumed_count < TOTAL_ITEM) {
            int data = ring_buffer[tail];
            tail = (tail + 1) % BUFFER_SIZE;
            total_consumed_count++;
            printf("[Consumer %d] Doc duoc: %d ( Tong tich luy: %d/15)\n", consumer_id, data, total_consumed_count);
        }

        xSemaphoreGive(xBufMutex);

        xSemaphoreGive(xEmptySlotsSem);

        vTaskDelay(pdMS_TO_TICKS(150));
    }

    printf("--> Consumer %d: Da xu li du lieu, dung tac vu.\n", consumer_id);
    vTaskDelete(NULL);
}

void app_main(void) {
    nvs_flash_init_in_main();

    // init_uart_with_queue();

    // xLogicQueue = xQueueCreate(10, sizeof(data_packet_t));

    // if (xLogicQueue != NULL) {
    //     xTaskCreate(uart_receiver_task, "Task 1", 4096, NULL, 12, NULL);
    //     xTaskCreate(logic_process_task, "Task 2", 2048, NULL, 10, NULL);   
    // }
    // else {
    //     ESP_LOGE("[Main]", "Failed to create Queue");
    // }

    printf("=== KHOI CHAY BAI TOAN PRODUCER - CONSUMER (MUTEX + SEMAPHORE) ===\n"); 

    xBufMutex = xSemaphoreCreateMutex();
    xEmptySlotsSem = xSemaphoreCreateCounting(BUFFER_SIZE, BUFFER_SIZE);
    xFilledSlotsSem = xSemaphoreCreateCounting(BUFFER_SIZE, 0);

    if (xBufMutex == NULL || xEmptySlotsSem == NULL || xFilledSlotsSem == NULL) {
        printf("Loi khoi tao Semaphore/Mutex!\n");
        return;
    }

    for (int i = 0; i < 2; i++) {
        char task_name[20];
        snprintf(task_name, sizeof(task_name), "Consumer_%d", i);
        xTaskCreate(consumer_task, task_name, 2048, (void*)i, 5, NULL);
    }

    for (int i = 0; i < 5; i++) {
        char task_name[20];
        snprintf(task_name, sizeof(task_name), "Producer_%d", i);
        xTaskCreate(producer_task, task_name, 2048, (void*)i, 5, NULL);
    }
}


