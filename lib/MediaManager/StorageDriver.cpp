#include "StorageDriver.h"
#include "../../include/config.h"
#include <SD.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

extern SemaphoreHandle_t spiMutex; // Lấy chìa khóa từ main.cpp

void StorageDriver::init() {
    // Xin quyền SPI để khởi tạo thẻ nhớ
    if (xSemaphoreTake(spiMutex, portMAX_DELAY) == pdTRUE) {
        if (!SD.begin(SD_CS_PIN)) {
            Serial.println("[LỖI] Không tìm thấy Thẻ nhớ SD!");
        } else {
            Serial.println("[HAL: Storage] Đã nhận Thẻ nhớ SD thành công.");
        }
        xSemaphoreGive(spiMutex);
    }
}

void StorageDriver::writeLog(const char* message) {
    // Xin quyền SPI trước khi ghi file
    if (xSemaphoreTake(spiMutex, 100 / portTICK_PERIOD_MS) == pdTRUE) {
        
        File file = SD.open("/syslog.txt", FILE_APPEND);
        if (file) {
            file.println(message);
            file.close();
            Serial.printf("[HAL: Storage] Đã ghi thẻ SD: %s\n", message);
        } else {
            Serial.println("[HAL: Storage] Lỗi mở file ghi log!");
        }

        xSemaphoreGive(spiMutex); // Trả quyền cho màn hình vẽ
    } else {
        Serial.println("[CẢNH BÁO] Storage không lấy được SPI Mutex!");
    }
}