#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "../../include/config.h"
#include "StorageDriver.h"

extern SPIClass hspi; // Lấy bus SPI từ main.cpp sang

bool Storage_Init() {
    Serial.println("[HAL] Dang khoi tao the SD...");
    if (!SD.begin(SD_CS, hspi)) {
        Serial.println("[HAL_Lỗi] Khong tim thay the SD!");
        return false;
    }
    return true;
}

void Storage_WriteLog(const char* message) {
    // Mở file syslog.txt ở chế độ Ghi nối tiếp (Append)
    File file = SD.open("/syslog.txt", FILE_APPEND);
    if(file) {
        file.println(message);
        file.close();
    } else {
        Serial.println("[HAL_Lỗi] Khong the ghi log vao the SD!");
    }
}