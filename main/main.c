// from: C
#include <string.h>
// from: ESP-IDF
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
// from: "annoying_default_funcs" folder
#include "nvs_init_in_main.h"
// project

bool task2_flag = false;
char buf[100] = {0};
int ind = 0;
void task_1(void *vParameter){
    int c;
    while (1) {
        c = getchar();
        if (c == EOF) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        if (c != '\n') {
            buf[ind++] = c;
            printf("%c", c);
        } 
        else {
            task2_flag = true;
        }
    }
}
void task_2(void *vParameter){
    while (1) {
        if (task2_flag) {
            printf("\nBuf = %s\n", buf);
            // reset state
            memset(buf, 0, sizeof(buf));
            task2_flag = false;
            ind = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void app_main(void) {
    nvs_flash_init_in_main();

    xTaskCreate(task_1, "Task 1", 4096, NULL, 4, NULL);
    xTaskCreate(task_2, "Task 2", 2048, NULL, 4, NULL);   
}


