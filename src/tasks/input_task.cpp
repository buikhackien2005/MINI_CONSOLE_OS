#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "../../include/config.h"

void InputTask(void *pvParameters) {
    Serial.println("[Task] InputTask (Core 1): Chuyển sang chế độ theo dõi Joystick SW.");

    while (1) {
        // Quét nút bấm SW của Joystick
        if (digitalRead(JOY_SW_PIN) == LOW) {
            Serial.println("[JOYSTICK] Đã nhấn nút SW!");
            // Đợi nhả nút để chống dội phím
            while(digitalRead(JOY_SW_PIN) == LOW) { vTaskDelay(10); } 
        }
        
        // Ngủ 50ms để nhường CPU
        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
}