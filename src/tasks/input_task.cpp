#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../../include/config.h"
#include "../../include/events.h"

// Khai báo Queue global
extern QueueHandle_t inputQueue;
extern QueueHandle_t renderQueue; // Phải khai báo extern ở đầu file
extern QueueHandle_t mediaQueue; // Thêm dòng này

// Hàm Ngắt (ISR)
void IRAM_ATTR isrButtonA() {
    static uint32_t last_isr_time = 0;
    uint32_t current_time = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS; 

    // Debounce 50ms
    if (current_time - last_isr_time > 50) { 
        InputEvent event = {1, true}; 
        BaseType_t xHigherPriorityTaskWoken = pdFALSE; 
        
        xQueueSendFromISR(inputQueue, &event, &xHigherPriorityTaskWoken);
        last_isr_time = current_time; 
        
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

// Thêm hàm này ngay dưới hàm isrButtonA()
void IRAM_ATTR isrButtonB() {
    static uint32_t last_isr_time_b = 0;
    uint32_t current_time = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS; 

    if (current_time - last_isr_time_b > 50) { 
        InputEvent event = {2, true}; // <-- NÚT B có ID là 2
        BaseType_t xHigherPriorityTaskWoken = pdFALSE; 
        
        xQueueSendFromISR(inputQueue, &event, &xHigherPriorityTaskWoken);
        last_isr_time_b = current_time; 
        
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

// Task xử lý Queue
void InputTask(void *pvParameters) {
    InputEvent receivedEvent;

    while (1) {
        if (xQueueReceive(inputQueue, &receivedEvent, portMAX_DELAY) == pdPASS) {
            if (receivedEvent.buttonID == 1) {
                Serial.println("[InputTask] -> Nút A vừa được nhấn!");
    
                // 1. Đẩy lệnh cho màn hình (Code cũ)
                DisplayEvent drawCmd = {1, 10, 20, "HELLO ESP32"}; 
                xQueueSend(renderQueue, &drawCmd, portMAX_DELAY); 

                // 2. Đẩy lệnh phát tiếng/nháy LED cho Core 0 (MỚI)
                MediaEvent beepCmd = {1, ""};
                xQueueSend(mediaQueue, &beepCmd, portMAX_DELAY);

                // 3. Đẩy lệnh ghi Log cho Core 0 (MỚI)
                MediaEvent logCmd = {2, "User bam nut A"};
                xQueueSend(mediaQueue, &logCmd, portMAX_DELAY);
            }
        
            // Thêm nhánh xử lý cho Nút B
            else if (receivedEvent.buttonID == 2) {
                Serial.println("[InputTask] -> Nút B vừa được nhấn! (Lệnh xóa)");

                // 1. Gửi lệnh Xóa màn hình sang Core 1 (cmdType = 0 là lệnh xóa đã định nghĩa ở Bước 1)
                DisplayEvent clearCmd = {0, 0, 0, ""}; 
                xQueueSend(renderQueue, &clearCmd, portMAX_DELAY); 

                // 2. Gửi lệnh ghi Log sang Core 0
                MediaEvent logCmd = {2, "User bam nut B"};
                xQueueSend(mediaQueue, &logCmd, portMAX_DELAY);
            }
        }
    }
}