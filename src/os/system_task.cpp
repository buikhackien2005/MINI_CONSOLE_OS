#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_sleep.h"
#include "../../include/events.h"
#include "../../include/config.h"
#include "../../lib/HAL_Audio/AudioDriver.h"
#include "../../lib/HAL_Storage/StorageDriver.h"

extern QueueHandle_t mediaQueue;
extern volatile unsigned long lastActivityTime;

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
        // Đợi lệnh trong 1 giây. Nếu hết 1 giây không có lệnh sẽ chạy xuống nhánh 'else'
        if (xQueueReceive(mediaQueue, &receivedEvent, 1000 / portTICK_PERIOD_MS) == pdPASS) {
            switch (receivedEvent.cmdType) {
                case 1: AudioDriver::playBeep(); break;
                case 2: StorageDriver::writeLog(receivedEvent.logData); break;
            }
        } 
        else {
            // Kiểm tra xem đã hết 30 giây chưa
            if (millis() - lastActivityTime > SLEEP_TIMEOUT) {
                Serial.println("\n[OS] Khong co hoat dong trong 30s. Di vao che do SLEEP...");
                
                // 1. Tắt đèn nền màn hình
                digitalWrite(TFT_BLK, LOW);
                
                // 2. Cho phép đánh thức bằng nút SW của Joystick
                gpio_wakeup_enable((gpio_num_t)JOY_SW_PIN, GPIO_INTR_LOW_LEVEL);
                esp_sleep_enable_gpio_wakeup();

                // 3. Đóng băng CPU (Ngủ nông)
                esp_light_sleep_start(); 

                // --- SAU KHI BẤM NÚT JOSTICK, CPU SẼ THỨC DẬY TẠI ĐÂY ---
                Serial.println("[OS] He thong da THUC DAY!");
                
                // 4. Bật lại màn hình và reset đồng hồ
                digitalWrite(TFT_BLK, HIGH);
                lastActivityTime = millis();
            }
        }
    }
}