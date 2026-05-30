#ifndef HAL_DISPLAY_H
#define HAL_DISPLAY_H

#include <stdint.h>

// Định nghĩa mã màu chung (App không cần biết TFT_eSPI dùng mã màu gì)
#define COLOR_BLACK  0x0000
#define COLOR_WHITE  0xFFFF
#define COLOR_YELLOW 0xFFE0
#define COLOR_BLUE   0x001F
#define COLOR_GREEN  0x07E0
#define COLOR_BG     0x0008 // Màu nền xanh đen của Game

void Display_Init();
void Display_FillScreen(uint16_t color);
void Display_DrawRect(int x, int y, int w, int h, uint16_t color);
void Display_DrawVLine(int x, int y, int h, uint16_t color);
void Display_DrawHLine(int x, int y, int w, uint16_t color);
void Display_DrawText(const char* text, int x, int y, int size, uint16_t color);
void Display_DrawInt(int num, int x, int y, int size, uint16_t color, uint16_t bg_color);

#endif