#include "InputManager.h"
#include "../../include/config.h"
#include "../../include/events.h"
#include "freertos/queue.h"

extern QueueHandle_t inputQueue;

// Mảng lưu thời gian bấm cuối cùng của 4 nút (Chống dội phím)
volatile unsigned long lastDebounceTime[5] = {0, 0, 0, 0, 0}; 

void InputManager::initPins() {
    // Chỉ cần khởi tạo nút nhấn SW tích hợp trên Joystick
    pinMode(JOY_SW_PIN, INPUT_PULLUP);
}