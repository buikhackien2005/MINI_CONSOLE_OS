#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <esp_task_wdt.h>
#include "../../include/events.h"
#include "../../lib/DisplayManager/DisplayManager.h"

extern QueueHandle_t renderQueue;
extern SemaphoreHandle_t spiMutex; // Lấy Mutex từ main.cpp sang

void DisplayTask(void *pvParameters) {
    DisplayEvent receivedEvent;

    // Lúc khởi tạo cũng đang dùng bus SPI, nên cũng phải xin khóa
    if (xSemaphoreTake(spiMutex, portMAX_DELAY) == pdTRUE) {
        DisplayManager::init();
        xSemaphoreGive(spiMutex); // Khởi tạo xong phải nhả khóa ngay
    }

    esp_task_wdt_add(NULL); 

    while (1) {
        if (xQueueReceive(renderQueue, &receivedEvent, 1000 / portTICK_PERIOD_MS) == pdPASS) {
            
            // XIN KHÓA MUTEX TRƯỚC KHI VẼ MÀN HÌNH
            // Chờ tối đa 100 tick (100ms). Nếu SystemTask đang dùng thẻ nhớ, task này sẽ bị Blocked chờ.
            if (xSemaphoreTake(spiMutex, 100 / portTICK_PERIOD_MS) == pdTRUE) {
                
                switch (receivedEvent.cmdType) {
                    case 0:
                        DisplayManager::clear();
                        break;
                    case 1:
                        DisplayManager::clear(); 
                        DisplayManager::drawText(receivedEvent.x, receivedEvent.y, receivedEvent.text);
                        break;
                }
                
                // VẼ XONG PHẢI TRẢ KHÓA LẠI CHO HỆ THỐNG
                xSemaphoreGive(spiMutex);
            } else {
                Serial.println("[CẢNH BÁO] DisplayTask không lấy được SPI Mutex (Bus đang bận)!");
            }
        }

        esp_task_wdt_reset(); 
    }
}