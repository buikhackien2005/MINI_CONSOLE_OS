#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../../include/events.h"
#include "../../lib/MediaManager/AudioDriver.h"
#include "../../lib/MediaManager/StorageDriver.h"

// Khai báo Queue cho Media
QueueHandle_t mediaQueue;

// Task xử lý hệ thống ngầm (Chạy trên Core 0)
void SystemTask(void *pvParameters) {
    MediaEvent receivedEvent;

    // Khởi tạo phần cứng liên quan đến Core 0
    AudioDriver::init();
    StorageDriver::init();

    while (1) {
        // Task ngủ chờ lệnh từ Queue
        if (xQueueReceive(mediaQueue, &receivedEvent, portMAX_DELAY) == pdPASS) {
            
            if (receivedEvent.cmdType == 1) {
                AudioDriver::playBeep();
            } 
            else if (receivedEvent.cmdType == 2) {
                StorageDriver::writeLog(receivedEvent.logData);
            }
        }
    }
}