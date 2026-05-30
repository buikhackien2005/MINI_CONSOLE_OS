#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_sleep.h"
#include "../../include/events.h"
#include "../../include/config.h"

// [MỚI] Sử dụng API của Tầng HAL
#include "../../lib/HAL_Audio/AudioDriver.h"
#include "../../lib/HAL_Storage/StorageDriver.h"

extern QueueHandle_t mediaQueue; 
extern volatile unsigned long lastActivityTime; 

void SystemTask(void *pvParameters) {
    MediaEvent receivedEvent;
    Serial.println("[OS] System Task (Core 0) dang chay ngam...");

    while (1) {
        // Lắng nghe Queue trong 1 giây
        if (xQueueReceive(mediaQueue, &receivedEvent, 1000 / portTICK_PERIOD_MS) == pdPASS) {
            // NẾU CÓ TIN NHẮN TỪ APP GỬI TỚI -> RA LỆNH CHO HAL THỰC THI
            switch (receivedEvent.cmdType) {
                case 1: 
                    Audio_PlayBeep(); 
                    break;
                case 2: 
                    Storage_WriteLog(receivedEvent.logData); 
                    break;
            }
        } 
        else {
            // NẾU KHÔNG CÓ TIN NHẮN -> KIỂM TRA SLEEP MODE TẮT MÀN HÌNH
            if (millis() - lastActivityTime > SLEEP_TIMEOUT) {
                Serial.println("\n[OS] He thong ranh roi 30s. Di vao che do SLEEP...");
                
                digitalWrite(TFT_BLK, LOW); // Tắt đèn nền
                
                gpio_wakeup_enable((gpio_num_t)JOY_SW_PIN, GPIO_INTR_LOW_LEVEL);
                esp_sleep_enable_gpio_wakeup();
                esp_light_sleep_start(); // Đóng băng CPU

                Serial.println("[OS] He thong da THUC DAY!");
                digitalWrite(TFT_BLK, HIGH); // Bật lại đèn nền
                lastActivityTime = millis();
            }
        }
    }
}