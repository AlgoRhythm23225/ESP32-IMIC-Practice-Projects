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
#include "ESP_MQTT.h"
#include "esp_log.h"
#include "esp_client_http.h"
#include "esp_crt_bundle.h"
#include "wifi_sta_ap.h"

// #include "driver/gpio.h"
// #include "driver/spi_master.h"

// #define PIN_NUM_MOSI    23
// #define PIN_NUM_CLK     18
// #define PIN_NUM_CS      5
// #define PIN_NUM_DC      2
// #define PIN_NUM_RST     4

// spi_device_handle_t tft_spi = NULL;

// void gpio_init_tft() {
//     gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
//     gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
// }

// void bus_init() {
//     spi_bus_config_t buscfg = {
//         .mosi_io_num = PIN_NUM_MOSI,
//         .miso_io_num = -1,
//         .sclk_io_num = PIN_NUM_CLK,
//         .quadhd_io_num = -1,
//         .quadwp_io_num = -1,
//     };

//     spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

//     spi_device_interface_config_t devcfg = {
//         .clock_speed_hz = 20 * 1000 * 1000,
//         .mode = 0,
//         .spics_io_num = PIN_NUM_CS,
//         .queue_size = 7,
//     };

//     spi_bus_add_device(SPI2_HOST, &devcfg, &tft_spi);
// }

// void lcd_cmd(uint8_t cmd) {
//     spi_transaction_t t;
//     memset(&t, 0, sizeof(t));

//     t.length = 8;
//     t.tx_buffer = &cmd;

//     gpio_set_level(PIN_NUM_DC, 0);

//     spi_device_transmit(tft_spi, &t);
// }

// void lcd_data(const uint8_t *data, int len) {
//     if (len == 0) {
//         return;
//     }

//     spi_transaction_t t;
//     memset(&t, 0, sizeof(t));

//     t.length = len * 8;
//     t.tx_buffer = data;

//     gpio_set_level(PIN_NUM_DC, 1);

//     spi_device_transmit(tft_spi, &t);
// }

// void lcd_reset() {
//     gpio_set_level(PIN_NUM_RST, 0);
//     vTaskDelay(pdMS_TO_TICKS(100));

//     gpio_set_level(PIN_NUM_RST, 1);
//     vTaskDelay(pdMS_TO_TICKS(100));
// }

// void st7735_init() {
//     gpio_init_tft();

//     lcd_reset();

//     // SWRESET 
//     lcd_cmd(0x01);
//     vTaskDelay(pdMS_TO_TICKS(150));
    
//     // SLPOUT
//     lcd_cmd(0x11);
//     vTaskDelay(pdMS_TO_TICKS(150));

//     // COLMOD = RGB565
//     lcd_cmd(0x3A);

//     uint8_t data = 0x05;

//     lcd_data(&data, 1);

//     // DISPON
//     lcd_cmd(0x29);
// }

// void st7735_set_window(
//     uint16_t x0,
//     uint16_t y0,
//     uint16_t x1,
//     uint16_t y1)
// {
//     uint8_t data[4];

//     // CASET
//     lcd_cmd(0x2A);

//     data[0] = x0 >> 8;
//     data[1] = x0 & 0xFF;
//     data[2] = x1 >> 8;
//     data[3] = x1 & 0xFF;

//     lcd_data(data, 4);

//     // RASET
//     lcd_cmd(0x2B);
//     data[0] = y0 >> 8;
//     data[1] = y0 & 0xFF;
//     data[2] = y1 >> 8;
//     data[3] = y1 & 0xFF;

//     lcd_data(data, 4);

//     // RAMWR
//     lcd_cmd(0x2C);
// } 

// typedef struct color {
//     uint8_t Red;
//     uint8_t Green;
//     uint8_t Blue;
// }color_t;

// uint16_t RGB_convert(uint8_t R, uint8_t G, uint8_t B) {
//     color_t color = {R, G, B};
//     return (color.Red >> 3) | (color.Green >> 2) << 5 | (color.Blue >> 3) << 11;
// }

// #define x1 103 // ofset = 24
// #define y1 159
// void st7735_fill(uint8_t R, uint8_t G, uint8_t B) {
//     st7735_set_window(24, 0, x1, y1);

//     uint8_t line[80 * 2];

//     uint16_t color = RGB_convert(R, G, B);
//     for (int i = 0; i < 80; i++) {
//         line[i * 2] = color >> 8;
//         line[i * 2 + 1] = color & 0xFF;
//     }

//     for (int y = 0; y < 160; y++) {
//         lcd_data(line, sizeof(line));
//     }
// }

// void st7735_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
//     if (x > x1 || y > y1) {
//         return;
//     }

//     st7735_set_window(x, y, x, y);

//     uint8_t data[2];
//     data[0] = color >> 8;
//     data[1] = color & 0xFF;

//     lcd_data(data, 2);
// }

// void st7735_draw_rectangle() {
//     uint8_t x = 24, y = 70;
//     uint8_t time = 5;
//     uint8_t clr = 0, clr2 = 0, clr3 = 0;
//         while (1) {
//             while(x < 89) {
//                 st7735_draw_pixel(x++, y, RGB_convert(clr, clr2, clr3));
//                 vTaskDelay(pdMS_TO_TICKS(time));
//             }
//             while(y < 135) {
//                 st7735_draw_pixel(x, y++, RGB_convert(clr, clr2, clr3));
//                 vTaskDelay(pdMS_TO_TICKS(time));
//             }
//             while(x > 24) {
//                 st7735_draw_pixel(x--, y, RGB_convert(clr, clr2, clr3));
//                 vTaskDelay(pdMS_TO_TICKS(time));
//             }
//             while(y > 70) {
//                 st7735_draw_pixel(x, y--, RGB_convert(clr, clr2, clr3));
//                 vTaskDelay(pdMS_TO_TICKS(time));
//             }
//             if (clr < 255) {
//                 clr += 15;
//             }
//             if (clr == 255 && clr2 < 255) {
//                 clr2 += 15;
//             }
//             if (clr == 255 && clr2 == 255 && clr3 < 255) {
//                 clr3 += 15;
//             }
//             if (clr == 255 && clr2 == 255 && clr3 == 255) {
//                 clr = clr2 = clr3 = 0;
//             }
//     }    
// }

// void st7735_fill_rect(uint16_t x, uint16_t y,
//                       uint16_t w, uint16_t h,
//                       uint16_t color)
// {
//     if (x + w > x1 + 1) w = (x1 + 1) - x;
//     if (y + h > y1 + 1) h = (y1 + 1) - y;

//     st7735_set_window(x, y, x + w - 1, y + h - 1);

//     uint8_t line[w * 2];

//     for (int i = 0; i < w; i++) {
//         line[i * 2] = color >> 8;
//         line[i * 2 + 1] = color & 0xFF;
//     }

//     for (int i = 0; i < h; i++) {
//         lcd_data(line, sizeof(line));
//     }
// }

// void st7735_draw_line(int x0, int y0, int x1, int y1, uint16_t color)
// {
//     int dx = abs(x1 - x0);
//     int sx = x0 < x1 ? 1 : -1;

//     int dy = -abs(y1 - y0);
//     int sy = y0 < y1 ? 1 : -1;

//     int err = dx + dy;

//     while (1) {
//         st7735_draw_pixel(x0, y0, color);

//         if (x0 == x1 && y0 == y1) break;

//         int e2 = 2 * err;

//         if (e2 >= dy) {
//             err += dy;
//             x0 += sx;
//         }

//         if (e2 <= dx) {
//             err += dx;
//             y0 += sy;
//         }
//     }
// }

void app_main(void) {
    nvs_flash_init_in_main();
    wifi_init_sta();
    WIFI_WAIT_CONNECT(wifi_event_group);

    mqtt_app_start();
    xTaskCreate(weather_task, "weather_task", 8192, NULL, 5, NULL);
    
}
// bus_init();
// st7735_init();
// st7735_fill(164, 119, 100);
// st7735_fill_rect(50, 60, 30, 70, RGB_convert(255, 0, 0));


