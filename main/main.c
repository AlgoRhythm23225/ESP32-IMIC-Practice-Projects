// from: C
#include <string.h>
// from: ESP-IDF
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
// from: "annoying_default_funcs" folder
#include "nvs_init_in_main.h"
// project

QueueHandle_t xMyQueue;

void task_1(void *vParameter){
    int c;
    int ind = 0;
    char *pBuffer = malloc(100);
    if (pBuffer == NULL) {
        vTaskDelete(NULL);
    }
    memset(pBuffer, 0, sizeof(pBuffer));

    while (1) {
        c = getchar();
        if (c == EOF) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        if (c != '\n'  && c != '\r') {
            if (ind < 99) {
                pBuffer[ind++] = c;
                printf("%c", c);
            }
        }
        else if (ind > 0) {
            pBuffer[ind] = '\0';
            printf("\n");
            if (xQueueSend(xMyQueue, &pBuffer, portMAX_DELAY) == pdPASS) {
                ESP_LOGI("[TASK 1]", "Sent string to Queue!");
            }
            else {
                free(pBuffer);
            }

            pBuffer = malloc(100);
            if (pBuffer == NULL) {
                ESP_LOGE("[Task 1]", "Not enough RAM for allocation!");
                vTaskDelete(NULL);
            }
            memset(pBuffer, 0, 100);
            ind = 0;
        } 
    }
}

void task_2(void *vParameter){
    char *pReceivedBuffer = NULL;

    while (1) {
        if (xQueueReceive(xMyQueue, &pReceivedBuffer, portMAX_DELAY) == pdPASS) {
            ESP_LOGI("[TASK 2]", "Received string: %s!", pReceivedBuffer);
            free(pReceivedBuffer);
            pReceivedBuffer = NULL;
        } 
    }
}

void app_main(void) {
    nvs_flash_init_in_main();

    xMyQueue = xQueueCreate(5, sizeof(int));

    if (xMyQueue != NULL) {
        xTaskCreate(task_1, "Task 1", 4096, NULL, 4, NULL);
        xTaskCreate(task_2, "Task 2", 2048, NULL, 4, NULL);   
    }
    else {
        ESP_LOGE("[Main]", "Failed to create Queue");
    }
}


