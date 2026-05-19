#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"

#define PIN_NUM_MOSI    23
#define PIN_NUM_CLK     18
#define PIN_NUM_CS      5
#define PIN_NUM_DC      2
#define PIN_NUM_RST     4

extern spi_device_handle_t tft_spi;

typedef struct color {
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
}color_t;

// Offset
#define x1_offset 103 // ofset = 24
#define y1_offset 159

void gpio_init_tft();
void bus_init();
void lcd_cmd(uint8_t cmd);
void lcd_data(const uint8_t *data, int len);
void lcd_reset();
void st7735_init();
void st7735_set_window (uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
uint16_t RGB_convert(uint8_t R, uint8_t G, uint8_t B);
void st7735_fill(uint8_t R, uint8_t G, uint8_t B);
void st7735_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void st7735_draw_line(int x0, int y0, int x1, int y1, uint16_t color);