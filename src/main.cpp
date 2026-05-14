#include <Arduino.h>
#include <esp_task_wdt.h>  // <-- BẮT BUỘC PHẢI CÓ DÒNG NÀY ĐỂ DÙNG WATCHDOG
#include "../../lib/InputManager/InputManager.h"
#include "../../include/config.h"
#include "../../include/events.h"

// --- KHAI BÁO BIẾN TOÀN CỤC & NGUYÊN MẪU ---
extern QueueHandle_t inputQueue;
extern QueueHandle_t renderQueue; // Thêm Queue hiển thị
extern QueueHandle_t mediaQueue; // Thêm dòng này

extern void isrButtonA();
extern void isrButtonB(); // <-- Khai báo thêm ngắt B
extern void InputTask(void *pvParameters);
extern void DisplayTask(void *pvParameters); // Thêm hàm Task hiển thị
extern void SystemTask(void *pvParameters); // Thêm dòng này

void setup() {
    Serial.begin(115200);

    // --- KHỞI TẠO WATCHDOG TIMER ---
    // Dùng cấu trúc chuẩn cho ESP32 Core v2.x: Timeout 3 giây, bắt buộc Panic (Reset) mạch
    esp_task_wdt_init(3, true); 
    // -------------------------------

    Serial.println("Khởi động Mini-Console OS...");

    // 1. Khởi tạo ngoại vi
    InputManager::initPins();

    // 2. Khởi tạo các Queue
    inputQueue = xQueueCreate(10, sizeof(InputEvent));
    renderQueue = xQueueCreate(5, sizeof(DisplayEvent)); 
    mediaQueue = xQueueCreate(10, sizeof(MediaEvent)); // Di chuyển lên gom chung cho gọn

    if (inputQueue == NULL || renderQueue == NULL || mediaQueue == NULL) {
        Serial.println("Lỗi tràn bộ nhớ: Không thể cấp phát Queue!");
        while (1); 
    }

    // 3. Gắn Ngắt
    attachInterrupt(digitalPinToInterrupt(BTN_A_PIN), isrButtonA, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_B_PIN), isrButtonB, FALLING); 

    // 4. Sinh ra các Task
    // Task hiển thị chạy trên Core 1, Priority 1 
    xTaskCreatePinnedToCore(
        DisplayTask, "DisplayTask", 4096, NULL, 1, NULL, 1 
    );

    // Task đọc phím chạy trên Core 1, Priority 2 (Quan trọng hơn)
    xTaskCreatePinnedToCore(
        InputTask, "InputTask", 2048, NULL, 2, NULL, 1 
    );

    // Task hệ thống âm thanh, lưu trữ chạy trên CORE 0
    xTaskCreatePinnedToCore(
        SystemTask, "SystemTask", 4096, NULL, 1, NULL, 0 
    );
}

void loop() {
    vTaskDelete(NULL); 
}