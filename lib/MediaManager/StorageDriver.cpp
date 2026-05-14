// --- StorageDriver.cpp ---
#include "StorageDriver.h"
#include "SPIFFS.h" // Thư viện quản lý file system nội bộ của ESP32

void StorageDriver::init() {
    // Tham số 'true' có nghĩa là: Nếu SPIFFS chưa được format, hãy format nó.
    if (!SPIFFS.begin(true)) {
        Serial.println("[LỖI] Không thể mount hệ thống file SPIFFS!");
        return;
    }
    Serial.println("[HAL: Storage] Đã khởi tạo ổ cứng ảo SPIFFS thành công.");
}

void StorageDriver::writeLog(const char* message) {
    // Mở file ở chế độ FILE_APPEND (Ghi nối tiếp vào cuối file)
    File file = SPIFFS.open("/syslog.txt", FILE_APPEND);
    if (!file) {
        Serial.println("[HAL: Storage] Lỗi mở file ghi log!");
        return;
    }
    file.println(message);
    file.close();
    Serial.printf("[HAL: Storage] Đã ghi vào Flash: %s\n", message);
}