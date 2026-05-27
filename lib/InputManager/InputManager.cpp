#include "InputManager.h"
#include "../../include/config.h"
#include "../../include/events.h"
#include "freertos/queue.h"

extern QueueHandle_t inputQueue;

// Mảng lưu thời gian bấm cuối cùng của 4 nút (Chống dội phím)
volatile unsigned long lastDebounceTime[5] = {0, 0, 0, 0, 0}; 

void InputManager::initPins() {
    pinMode(BTN_UP_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    pinMode(BTN_LEFT_PIN, INPUT_PULLUP);
    pinMode(BTN_RIGHT_PIN, INPUT_PULLUP);
}

// Hàm xử lý ngắt thông minh
void IRAM_ATTR isrSendEvent(uint8_t btnID) {
    unsigned long currentTime = millis();
    
    // NẾU 2 LẦN BẤM CÁCH NHAU DƯỚI 250ms -> ĐÓ LÀ NHIỄU, BỎ QUA!
    if ((currentTime - lastDebounceTime[btnID]) > 250) { 
        InputEvent event;
        event.buttonID = btnID;
        
        // Chỉ gửi vào Queue nếu qua được màng lọc nhiễu
        xQueueSendFromISR(inputQueue, &event, NULL);
        
        // Cập nhật lại thời gian
        lastDebounceTime[btnID] = currentTime;
    }
}

void IRAM_ATTR isrButtonUp()    { isrSendEvent(1); }
void IRAM_ATTR isrButtonDown()  { isrSendEvent(2); }
void IRAM_ATTR isrButtonLeft()  { isrSendEvent(3); }
void IRAM_ATTR isrButtonRight() { isrSendEvent(4); }