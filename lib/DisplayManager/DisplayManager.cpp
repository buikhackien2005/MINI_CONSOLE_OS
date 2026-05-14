#include "DisplayManager.h"
#include "../../include/config.h"

// Khởi tạo đối tượng màn hình I2C ở phạm vi global của file này
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void DisplayManager::init() {
    // 1. Khởi tạo chuẩn I2C với 2 chân đã cấu hình
    Wire.begin(OLED_SDA, OLED_SCL);

    // 2. Kích hoạt màn hình (Địa chỉ I2C mặc định thường là 0x3C)
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("[LỖI] Không tìm thấy màn hình OLED SSD1306!");
        for(;;); // Sập hệ thống nếu không có màn hình
    }

    // 3. Xóa rác đồ họa trong RAM màn hình, thiết lập màu chữ
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.display();
    
    Serial.println("[HAL: Display] Đã khởi tạo OLED 128x64 thành công.");
}

void DisplayManager::clear() {
    display.clearDisplay(); // Xóa buffer
}

void DisplayManager::drawText(uint8_t x, uint8_t y, const char* text) {
    display.setCursor(x, y);
    display.print(text); // Viết chữ vào buffer
}

void DisplayManager::update() {
    display.display(); // Bắn buffer qua I2C lên màn hình vật lý
}