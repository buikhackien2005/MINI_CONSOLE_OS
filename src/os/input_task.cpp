#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../include/config.h"
#include "../../lib/HAL_Input/InputDriver.h" // [MỚI]

extern volatile unsigned long lastActivityTime;
extern volatile int system_state;
extern volatile bool menu_selected;

void InputTask(void *pvParameters) {
    while (1) {
        // [ĐÃ SỬA] Dùng API thay vì digitalRead
        if (Input_IsHomePressed()) {
            lastActivityTime = millis(); 
            unsigned long pressTime = millis();
            
            while(Input_IsHomePressed()) { vTaskDelay(10); } 
            
            unsigned long releaseTime = millis();
            unsigned long holdDuration = releaseTime - pressTime;

            if (holdDuration > 1000) {
                if (system_state != 0) {
                    Serial.println("[OS] Nhan lenh HOME. Quay ve Menu!");
                    system_state = 0; 
                }
            } 
            else if (holdDuration > 50) {
                if (system_state == 0 || system_state == 2 || system_state == 3) menu_selected = true; 
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
}