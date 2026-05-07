#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../../include/config.h"
#include "../../include/events.h"

// 1. Khai báo Queue global cho module input
QueueHandle_t inputQueue;

// 2. Viết hàm Ngắt (ISR) cho Nút A với cơ chế Debounce
void IRAM_ATTR isrButtonA() {
    static uint32_t last_isr_time = 0;
    // Lấy thời gian thực của RTOS để chống dội phím (Debounce)
    uint32_t current_time = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS; 

    // Bỏ qua nếu lần ngắt trước đó cách hiện tại < 50ms (dội phím vật lý)
    if (current_time - last_isr_time > 50) { 
        InputEvent event = {1, true}; // Tạo gói tin: Nút 1 (A) được nhấn
        
        // Khai báo cờ yêu cầu chuyển ngữ cảnh (Context Switch)
        BaseType_t xHigherPriorityTaskWoken = pdFALSE; 
        
        // Ném gói tin vào Queue từ bên trong Ngắt
        xQueueSendFromISR(inputQueue, &event, &xHigherPriorityTaskWoken);
        
        last_isr_time = current_time; // Cập nhật thời gian
        
        // Ép RTOS chuyển sang chạy InputTask ngay lập tức nếu nó có Priority cao hơn
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

// 3. Viết Task xử lý sự kiện
void InputTask(void *pvParameters) {
    InputEvent receivedEvent;

    // Vòng lặp vô tận của Task
    while (1) {
        // Task sẽ "ngủ" (Blocked) ở đây cho đến khi Queue có dữ liệu
        if (xQueueReceive(inputQueue, &receivedEvent, portMAX_DELAY) == pdPASS) {
            // Khi có dữ liệu, thức dậy và in log
            if (receivedEvent.buttonID == 1) {
                Serial.println("[InputTask] -> Nút A vừa được nhấn! (Đã debounce)");
            }
        }
    }
}