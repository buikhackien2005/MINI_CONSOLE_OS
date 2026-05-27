#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../../include/events.h"
#include "../../lib/MediaManager/AudioDriver.h"
#include "../../lib/MediaManager/StorageDriver.h"

// Lấy hàng đợi đa phương tiện. ĐÃ XÓA spiMutex!
extern QueueHandle_t mediaQueue; 

void SystemTask(void *pvParameters) {
    Serial.println("[Task] SystemTask (Core 0) đang khởi động...");
    
    // Khởi tạo Amply và Thẻ nhớ
    AudioDriver::init();
    StorageDriver::init();

    MediaEvent receivedEvent;

    while (1) {
        // Đứng chờ (Ngủ) cho đến khi có lệnh hệ thống
        if (xQueueReceive(mediaQueue, &receivedEvent, portMAX_DELAY) == pdPASS) {
            
            // Xử lý lệnh MÀ KHÔNG CẦN XIN MUTEX nữa
            switch (receivedEvent.cmdType) {
                case 1: // Lệnh 1: Phát tiếng Bíp
                    AudioDriver::playBeep();
                    break;
                
                case 2: // Lệnh 2: Ghi file Log
                    // Gọi hàm ghi log của StorageDriver (nó sẽ tự đẩy qua đường HSPI)
                    StorageDriver::writeLog(receivedEvent.logData);
                    break;
            }
        }
    }
}