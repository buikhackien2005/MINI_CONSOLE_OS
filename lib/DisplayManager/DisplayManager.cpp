#include "DisplayManager.h"
#include "bitmaps.h"

// Khởi tạo object tft (Chân cẳng đã được cấu hình tự động từ platformio.ini)
TFT_eSPI tft = TFT_eSPI(); 

void DisplayManager::init() {
    tft.init();
    tft.setRotation(2); // Xoay ngang màn hình (Landcape)
    tft.fillScreen(TFT_BLACK); // Tô đen toàn bộ màn hình
    tft.setTextColor(TFT_WHITE, TFT_BLACK); // Chữ trắng, nền đen (giúp đè chữ cũ không bị lem)
    tft.setTextSize(2); // Cỡ chữ lớn hơn vì độ phân giải cao hơn
    
    Serial.println("[HAL: Display] Đã khởi tạo màn hình TFT 1.77 Inch (SPI).");
}

void DisplayManager::clear() {
    tft.fillScreen(TFT_BLACK); // Việc tô 40KB màu đen trực tiếp qua SPI sẽ mất khoảng 10-20ms
}

void DisplayManager::drawText(uint8_t x, uint8_t y, const char* text) {
    tft.setCursor(x, y);
    tft.print(text); 
}

// CÀI ĐẶT HÀM VẼ ẢNH:
void DisplayManager::drawImage() {
    // [BEGIN lopaka generated]
    tft.drawBitmap(0, 0, image_paint_1_bits, 128, 160, 0x7FC);
    tft.fillEllipse(76, 22, 4, 4, 0x0);
    tft.fillEllipse(75, 142, 4, 4, 0x0);
    tft.drawBitmap(46, 41, image_paint_4_bits, 23, 42, 0x0);
    tft.drawBitmap(46, 80, image_paint_4_copy_1_bits, 23, 42, 0x0);
    tft.drawBitmap(47, 44, image_paint_6_bits, 21, 76, 0x0);
    // [END lopaka generated]
}