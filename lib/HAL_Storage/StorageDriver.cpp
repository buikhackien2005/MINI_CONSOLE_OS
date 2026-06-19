#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <LittleFS.h> 
#include "../../include/config.h"
#include "StorageDriver.h"
#include "../HAL_Display/DisplayDriver.h" // [SỬA Ở ĐÂY 1] Thêm dòng này để dùng hàm Display_SetBrightness()

extern SPIClass hspi;
extern int max_score;             
extern float base_paddle_speed;
extern int system_brightness;     // [SỬA Ở ĐÂY 2] Thêm dòng này để mượn biến độ sáng từ main.cpp sang

// [MỚI] Cờ trạng thái theo dõi xem thẻ SD có đang được cắm không
bool is_sd_mounted = false;

// ==========================================
// 1. KHỞI TẠO ROOT FS (Ổ ĐĨA NỘI BỘ)
// ==========================================
bool Storage_Init_RootFS() {
    Serial.println("[HAL] Dang mount Root File System (LittleFS)...");
    if (!LittleFS.begin(true)) { 
        Serial.println("[HAL_Lỗi] Khong the mount LittleFS!");
        return false;
    }
    Serial.println("[HAL] LittleFS Mount thanh cong! (Mount point: / )");

    if (!LittleFS.exists("/etc")) {
        LittleFS.mkdir("/etc");
    }

    if (!LittleFS.exists("/etc/config.txt")) {
        File f = LittleFS.open("/etc/config.txt", FILE_WRITE);
        if (f) {
            f.println("MAX_SCORE=7");
            f.println("PADDLE_SPEED=2.0");
            f.close();
        }
    }
    return true;
}

// ==========================================
// 2. KHỞI TẠO SD CARD (Ổ CẮM NGOÀI)
// ==========================================
bool Storage_Init_SD() {
    Serial.println("[HAL] Dang mount the SD (/sdcard)...");
    if (!SD.begin(SD_CS, hspi)) {
        Serial.println("[HAL_Cảnh báo] Khong tim thay the SD! (OS van se chay bang RootFS)");
        is_sd_mounted = false; // Đánh dấu là KHÔNG có thẻ
        return false;
    }
    Serial.println("[HAL] The SD Mount thanh cong!");
    is_sd_mounted = true; // Đánh dấu là CÓ thẻ
    return true;
}

// ==========================================
// 3. ĐỌC CẤU HÌNH TỪ ROOT FS
// ==========================================
// Cập nhật hàm Load để đọc thêm dòng BRIGHTNESS
void Storage_LoadConfig() {
    File f = LittleFS.open("/etc/config.txt", FILE_READ);
    if (!f) return;
    
    Serial.println("[OS] Dang nap cau hinh tu Root FS...");
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.startsWith("MAX_SCORE=")) max_score = line.substring(10).toInt();
        else if (line.startsWith("PADDLE_SPEED=")) base_paddle_speed = line.substring(13).toFloat();
        else if (line.startsWith("BRIGHTNESS=")) system_brightness = line.substring(11).toInt();
    }
    f.close();
    
    // Áp dụng ngay độ sáng vừa đọc được
    Display_SetBrightness(system_brightness);
}

// [MỚI] Viết hàm Lưu file đè lên RootFS
void Storage_SaveConfig() {
    File f = LittleFS.open("/etc/config.txt", FILE_WRITE);
    if (f) {
        f.printf("MAX_SCORE=%d\n", max_score);
        f.printf("PADDLE_SPEED=%.1f\n", base_paddle_speed);
        f.printf("BRIGHTNESS=%d\n", system_brightness);
        f.close();
        Serial.println("[OS] Da luu cau hinh vao RootFS!");
    } else {
        Serial.println("[OS_Lỗi] Khong the ghi file config.txt");
    }
}

// ==========================================
// 4. GHI LOG THÔNG MINH (CHỐNG LỖI VFS)
// ==========================================
void Storage_WriteLog(const char* message) {
    // [MỚI] Chỉ cố gắng mở thẻ SD nếu cờ trạng thái báo là có thẻ!
    if (is_sd_mounted) {
        File file = SD.open("/syslog.txt", FILE_APPEND);
        if(file) {
            file.println(message);
            file.close();
            return; // Ghi xong thì thoát hàm luôn
        }
    }
    
    // Nếu không có thẻ SD (hoặc SD bị lỗi), ghi thẳng vào LittleFS
    File rootFile = LittleFS.open("/syslog.txt", FILE_APPEND);
    if (rootFile) {
        rootFile.println(message);
        rootFile.close();
    }
}

// ==========================================
// 5. CẢM BIẾN HOT-SWAP (CẮM RÚT NÓNG)
// ==========================================
int Storage_CheckHotSwap() {
    if (is_sd_mounted) {
        // Đang cắm -> Ép ESP32 "gõ cửa" đọc vật lý thẻ nhớ
        File test_read = SD.open("/");
        if (!test_read) {
            // Mở thất bại -> Thẻ đã bị rút!
            SD.end(); // Dọn dẹp đường truyền SPI
            is_sd_mounted = false;
            return -1; // Trả về tín hiệu VỪA RÚT THẺ
        }
        test_read.close(); // Mở thành công thì đóng lại ngay
    } else {
        // Chưa cắm -> Thử kết nối lại
        if (SD.begin(SD_CS, hspi)) {
            is_sd_mounted = true;
            return 1; // Trả về tín hiệu VỪA CẮM THẺ
        }
    }
    return 0; // Trạng thái không đổi
}