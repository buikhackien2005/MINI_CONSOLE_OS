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
            if (receivedEvent.buttonID == 1) { // BẤM NÚT A
            Serial.println("[InputTask] -> Nút A: Yêu cầu vẽ ảnh!");
            
            // Đổi cmdType thành 2, không cần truyền text hay tọa độ nữa
            DisplayEvent drawCmd = {2, 0, 0, ""}; 
            xQueueSend(renderQueue, &drawCmd, portMAX_DELAY); 
            
            // Vẫn phát tiếng Bíp và ghi Log nếu bạn muốn
            MediaEvent beepCmd = {1, ""};
            xQueueSend(mediaQueue, &beepCmd, portMAX_DELAY);
            MediaEvent logCmd = {2, "Ve anh Lopaka"};
            xQueueSend(mediaQueue, &logCmd, portMAX_DELAY);
        }
        
        else if (receivedEvent.buttonID == 2) { // BẤM NÚT B
            Serial.println("[InputTask] -> Nút B: Xóa màn hình!");

            // cmdType = 0 đã được quy định là lệnh xóa
            DisplayEvent clearCmd = {0, 0, 0, ""}; 
            xQueueSend(renderQueue, &clearCmd, portMAX_DELAY); 

            MediaEvent logCmd = {2, "Xoa man hinh"};
            xQueueSend(mediaQueue, &logCmd, portMAX_DELAY);
        }
        }
    }
}