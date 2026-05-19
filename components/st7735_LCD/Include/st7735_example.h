#ifndef ST7735_DRAW_H
#define ST7735_DRAW_H

#include "st7735.h"

void st7735_draw_rectangle();
void st7735_fill_rect(uint16_t x, uint16_t y,
                      uint16_t w, uint16_t h,
                      uint16_t color);

#endif