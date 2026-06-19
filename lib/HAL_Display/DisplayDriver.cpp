#include <TFT_eSPI.h>
#include "DisplayDriver.h"
#include "../../include/config.h"

// Biến tft vật lý được giam lỏng ở đây, không cho thoát ra ngoài!
TFT_eSPI tft = TFT_eSPI();

void Display_Init() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(COLOR_BLACK);

    // [MỚI] Cấu hình PWM cho Đèn nền màn hình (Kênh 0, Tần số 5000Hz, Phân giải 8-bit)
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BLK, 0);
}

void Display_SetBrightness(int percent) {
    // [MỚI] Ngoại lệ đặc biệt: Số 0 có nghĩa là tắt hẳn màn hình (dành cho Sleep Mode)
    if (percent == 0) {
        ledcWrite(0, 0); 
        return;
    }
    
    // Giới hạn cho người dùng (10% - 100%)
    if (percent < 10) percent = 10;   
    if (percent > 100) percent = 100;
    
    // Đổi từ phần trăm sang thang đo 0-255 của PWM 8-bit
    int duty_cycle = (percent * 255) / 100; 
    ledcWrite(0, duty_cycle);
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

void Display_FillRect(int x, int y, int w, int h, uint16_t color) {
    tft.fillRect(x, y, w, h, color);
}

void Display_DrawTaskbar(int hour, int minute, bool sd_ok) {
    // 1. Vẽ trạng thái thẻ SD ở góc Trái (Không vẽ nền nữa)
    if (sd_ok) {
        Display_DrawText("SD", 5, 118, 1, COLOR_GREEN);
    } else {
        Display_DrawText("NO SD", 5, 118, 1, COLOR_RED);
    }

    // 2. Vẽ Đồng hồ ở góc Phải
    char time_str[10];
    sprintf(time_str, "%02d:%02d", hour, minute); 
    Display_DrawText(time_str, 125, 118, 1, COLOR_WHITE); 
}