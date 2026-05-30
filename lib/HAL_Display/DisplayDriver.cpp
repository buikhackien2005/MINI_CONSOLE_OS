#include <TFT_eSPI.h>
#include "DisplayDriver.h"

// Biến tft vật lý được giam lỏng ở đây, không cho thoát ra ngoài!
TFT_eSPI tft = TFT_eSPI();

void Display_Init() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(COLOR_BLACK);
}

void Display_FillScreen(uint16_t color) { tft.fillScreen(color); }
void Display_DrawRect(int x, int y, int w, int h, uint16_t color) { tft.fillRect(x, y, w, h, color); }
void Display_DrawVLine(int x, int y, int h, uint16_t color) { tft.drawFastVLine(x, y, h, color); }
void Display_DrawHLine(int x, int y, int w, uint16_t color) { tft.drawFastHLine(x, y, w, color); }

void Display_DrawText(const char* text, int x, int y, int size, uint16_t color) {
    tft.setTextColor(color); tft.setTextSize(size);
    tft.setCursor(x, y); tft.print(text);
}

void Display_DrawInt(int num, int x, int y, int size, uint16_t color, uint16_t bg_color) {
    tft.setTextColor(color, bg_color); tft.setTextSize(size);
    tft.setCursor(x, y); tft.printf("%d", num);
}