#include "StorageDriver.h"
#include "../../include/config.h"
#include <SD.h>
#include <SPI.h>

extern SPIClass hspi; // Lấy luồng HSPI từ main.cpp

void StorageDriver::init() {
    // Ép Thẻ nhớ dùng luồng hspi
    if (!SD.begin(SD_CS, hspi)) { 
        Serial.println("[LỖI] Không tìm thấy Thẻ SD!");
    } else {
        //Serial.println("[HAL: Storage] Đã nhận Thẻ nhớ SD trên HSPI.");
    }
}

void StorageDriver::writeLog(const char* message) {
    // KHÔNG CẦN MUTEX NỮA! Ghi thẳng vào thẻ!
    File file = SD.open("/syslog.txt", FILE_APPEND);
    if (file) {
        file.println(message);
        file.close();
        //Serial.printf("[HAL: Storage] Đã ghi thẻ SD: %s\n", message);
    }
}