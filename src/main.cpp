#include <Arduino.h>
#include "../../lib/InputManager/InputManager.h"
#include "../../include/config.h"

// Khai báo nguyên mẫu (prototype) từ file input_task.cpp
extern QueueHandle_t inputQueue;
extern void isrButtonA();
extern void InputTask(void *pvParameters);

void setup() {
    Serial.begin(115200);
    Serial.println("Khởi động Mini-Console OS...");

    // 1. Khởi tạo chân phần cứng
    InputManager::initPins();

    // 2. Khởi tạo Queue (sức chứa 10 sự kiện)
    inputQueue = xQueueCreate(10, sizeof(InputEvent));
    if (inputQueue == NULL) {
        Serial.println("Lỗi: Không thể cấp phát bộ nhớ cho inputQueue!");
        while (1); // Dừng hệ thống nếu lỗi
    }

    // 3. Gắn Ngắt (Interrupt) vào chân vật lý
    // Dùng FALLING vì thiết lập ban đầu là INPUT_PULLUP (HIGH). Nhấn xuống thành LOW.
    attachInterrupt(digitalPinToInterrupt(BTN_A_PIN), isrButtonA, FALLING);

    // 4. Tạo FreeRTOS Task và ép chạy trên Core 1
    xTaskCreatePinnedToCore(
        InputTask,      // Tên hàm thực thi
        "InputTask",    // Tên Task (để debug)
        2048,           // Kích thước Stack (2KB là dư dả cho task này)
        NULL,           // Tham số truyền vào (không có)
        2,              // Mức độ ưu tiên (Priority)
        NULL,           // Task Handle (không cần dùng lúc này)
        1               // Ép chạy trên Core 1
    );
}

void loop() {
    // Xóa file loop() mặc định. Trong RTOS, ta không dùng loop() nữa.
    vTaskDelete(NULL); 
}