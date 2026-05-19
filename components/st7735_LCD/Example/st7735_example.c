#include "st7735_example.h"

void st7735_draw_rectangle() {
    uint8_t x = 24, y = 70;
    uint8_t time = 5;
    uint8_t clr = 0, clr2 = 0, clr3 = 0;
        while (1) {
            while(x < 89) {
                st7735_draw_pixel(x++, y, RGB_convert(clr, clr2, clr3));
                vTaskDelay(pdMS_TO_TICKS(time));
            }
            while(y < 135) {
                st7735_draw_pixel(x, y++, RGB_convert(clr, clr2, clr3));
                vTaskDelay(pdMS_TO_TICKS(time));
            }
            while(x > 24) {
                st7735_draw_pixel(x--, y, RGB_convert(clr, clr2, clr3));
                vTaskDelay(pdMS_TO_TICKS(time));
            }
            while(y > 70) {
                st7735_draw_pixel(x, y--, RGB_convert(clr, clr2, clr3));
                vTaskDelay(pdMS_TO_TICKS(time));
            }
            if (clr < 255) {
                clr += 15;
            }
            if (clr == 255 && clr2 < 255) {
                clr2 += 15;
            }
            if (clr == 255 && clr2 == 255 && clr3 < 255) {
                clr3 += 15;
            }
            if (clr == 255 && clr2 == 255 && clr3 == 255) {
                clr = clr2 = clr3 = 0;
            }
    }    
}

void st7735_fill_rect(uint16_t x, uint16_t y,
                      uint16_t w, uint16_t h,
                      uint16_t color)
{
    if (x + w > x1_offset + 1) w = (x1_offset + 1) - x;
    if (y + h > y1_offset + 1) h = (y1_offset + 1) - y;

    st7735_set_window(x, y, x + w - 1, y + h - 1);

    uint8_t line[w * 2];

    for (int i = 0; i < w; i++) {
        line[i * 2] = color >> 8;
        line[i * 2 + 1] = color & 0xFF;
    }

    for (int i = 0; i < h; i++) {
        lcd_data(line, sizeof(line));
    }
}