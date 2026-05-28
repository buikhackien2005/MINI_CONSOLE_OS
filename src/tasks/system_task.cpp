#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h" // [MỚI] Thư viện để đo RAM
#include "../../include/events.h"
#include "../../lib/MediaManager/AudioDriver.h"
#include "../../lib/MediaManager/StorageDriver.h"

extern QueueHandle_t mediaQueue; 

// [MỚI] Lấy danh sách Thẻ căn cước từ main.cpp
extern TaskHandle_t displayTaskHandle;
extern TaskHandle_t inputTaskHandle;
extern TaskHandle_t systemTaskHandle;

void SystemTask(void *pvParameters) {
    Serial.println("[Task] SystemTask (Core 0) đang khởi động...");
    
    AudioDriver::init();
    StorageDriver::init();

    MediaEvent receivedEvent;

    while (1) {
        // Trở lại trạng thái ngủ đông hoàn toàn (portMAX_DELAY)
        if (xQueueReceive(mediaQueue, &receivedEvent, portMAX_DELAY) == pdPASS) {
            switch (receivedEvent.cmdType) {
                case 1: AudioDriver::playBeep(); break;
                case 2: StorageDriver::writeLog(receivedEvent.logData); break;
            }
        } 
    }
}