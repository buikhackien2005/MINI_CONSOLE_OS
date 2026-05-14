#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <esp_task_wdt.h> // <-- Thư viện Watchdog
#include "../../include/events.h"
#include "../../lib/DisplayManager/DisplayManager.h"

QueueHandle_t renderQueue;

void DisplayTask(void *pvParameters) {
    DisplayEvent receivedEvent;

    DisplayManager::init();

    // Đăng ký Task này với hệ thống Watchdog
    esp_task_wdt_add(NULL); 

    while (1) {
        // Thay vì chờ vô hạn (portMAX_DELAY), ta chỉ chờ tối đa 1 giây (1000 tick)
        if (xQueueReceive(renderQueue, &receivedEvent, 1000 / portTICK_PERIOD_MS) == pdPASS) {
            
            switch (receivedEvent.cmdType) {
                case 0:
                    DisplayManager::clear();
                    DisplayManager::update();
                    break;
                case 1:
                    DisplayManager::clear(); 
                    DisplayManager::drawText(receivedEvent.x, receivedEvent.y, receivedEvent.text);
                    DisplayManager::update(); 
                    break;
            }
            
            // In Profiling
            UBaseType_t ramDu = uxTaskGetStackHighWaterMark(NULL);
            Serial.printf("[Profiler] DisplayTask RAM dư: %d Words\n", ramDu);
        }

        // Bất kể có nhận được lệnh vẽ hay không, sau mỗi 1 giây task đều lọt xuống đây
        // Gọi hàm reset (feed the dog) để báo cho TWDT biết Task này vẫn đang sống khỏe
        esp_task_wdt_reset(); 
    }
}