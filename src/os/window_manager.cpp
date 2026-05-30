#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "../../include/config.h"
#include "../../lib/HAL_Display/DisplayDriver.h"
#include "../../lib/HAL_Input/InputDriver.h"
#include "../apps/app_pingpong.h"

extern volatile int system_state;
extern volatile bool menu_selected;
extern volatile unsigned long lastActivityTime;

int menu_index = 0; 
bool force_redraw_menu = true;

void drawMenu() {
    Display_FillScreen(COLOR_BLACK);
    Display_DrawText("MINI OS", 20, 10, 2, COLOR_GREEN);
    Display_DrawHLine(0, 30, 160, COLOR_GREEN);

    // 0x8410 là mã màu Xám đậm (Gray)
    if (menu_index == 0) { 
        Display_DrawRect(10, 45, 140, 20, COLOR_BLUE); 
        Display_DrawText("> 1. Game Ping Pong", 15, 50, 1, COLOR_WHITE);
    } else {
        Display_DrawText("> 1. Game Ping Pong", 15, 50, 1, 0x8410); 
    }

    if (menu_index == 1) { 
        Display_DrawRect(10, 75, 140, 20, COLOR_BLUE); 
        Display_DrawText("> 2. Music Player", 15, 80, 1, COLOR_WHITE);
    } else {
        Display_DrawText("> 2. Music Player", 15, 80, 1, 0x8410);
    }
    
    Display_DrawText("Giu 'SW' 1.5s de Thoat", 10, 110, 1, COLOR_YELLOW);
}

void WindowManagerTask(void *pvParameters) {
    Serial.println("[OS] Window Manager khoi dong...");

    while (1) {
        if (system_state == 0) {
            if (force_redraw_menu) { drawMenu(); force_redraw_menu = false; }

            // [ĐÃ SỬA] Đọc Joystick qua API
            int joyY = Input_GetJoyY();
            if (joyY < 1000 && menu_index > 0) {
                menu_index--; drawMenu(); lastActivityTime = millis();
                vTaskDelay(200 / portTICK_PERIOD_MS); 
            } else if (joyY > 3000 && menu_index < 1) {
                menu_index++; drawMenu(); lastActivityTime = millis();
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }

            if (menu_selected) {
                menu_selected = false;
                if (menu_index == 0) {
                    Serial.println("[OS] Launching Ping Pong App...");
                    system_state = 1; 
                    AppPingPong_Start(); 
                } else if (menu_index == 1) {
                    Display_FillScreen(COLOR_BG);
                    Display_DrawText("MP3 PLAYER", 25, 40, 2, COLOR_WHITE);
                    system_state = 2; 
                }
            }
        } 
        
        static int last_system_state = 0;
        if (system_state != last_system_state) {
            if (system_state == 0) {
                force_redraw_menu = true; 
                menu_index = 0; 
            }
            last_system_state = system_state; 
        }
        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
}