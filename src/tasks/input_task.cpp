#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../include/config.h"

extern volatile unsigned long lastActivityTime;
extern volatile int system_state;
extern volatile bool menu_selected;

void InputTask(void *pvParameters) {
    while (1) {
        if (digitalRead(JOY_SW_PIN) == LOW) {
            lastActivityTime = millis(); 
            unsigned long pressTime = millis();
            
            // Chờ người dùng nhả tay ra
            while(digitalRead(JOY_SW_PIN) == LOW) { vTaskDelay(10); } 
            
            unsigned long releaseTime = millis();
            unsigned long holdDuration = releaseTime - pressTime;

            if (holdDuration > 1000) {
                // BẤM GIỮ HƠN 1 GIÂY -> LỆNH HOME (VỀ MENU)
                if (system_state != 0) {
                    Serial.println("[OS] Nhận lệnh HOME. Đóng App, quay về Menu!");
                    system_state = 0; // Đổi trạng thái về Menu
                }
            } 
            else if (holdDuration > 50) {
                // BẤM NGẮN (CHỐNG NHIỄU 50ms) -> LỆNH SELECT (CHỌN APP)
                if (system_state == 0) {
                    menu_selected = true; 
                }
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
}