#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "../../include/config.h"
#include "../../lib/HAL_Display/DisplayDriver.h"
#include "../../lib/HAL_Input/InputDriver.h"
#include "../apps/app_pingpong.h"
#include "../apps/app_settings.h"

extern volatile int system_state;
extern volatile bool menu_selected;
extern volatile unsigned long lastActivityTime;
extern volatile bool request_full_redraw;

int menu_index = 0; 
bool force_redraw_menu = true;

void drawMenu() {
    Display_FillScreen(COLOR_BLACK);
    Display_DrawText("MINI OS", 20, 5, 2, COLOR_GREEN); 
    Display_DrawHLine(0, 25, 160, COLOR_GREEN);         

    if (menu_index == 0) { 
        Display_DrawRect(10, 35, 140, 20, COLOR_BLUE); 
        Display_DrawText("> 1. Game Ping Pong", 15, 40, 1, COLOR_WHITE);
    } else Display_DrawText("> 1. Game Ping Pong", 15, 40, 1, 0x8410); 

    if (menu_index == 1) { 
        Display_DrawRect(10, 60, 140, 20, COLOR_BLUE); 
        Display_DrawText("> 2. Music Player", 15, 65, 1, COLOR_WHITE);
    } else Display_DrawText("> 2. Music Player", 15, 65, 1, 0x8410);
    
    if (menu_index == 2) { 
        Display_DrawRect(10, 85, 140, 20, COLOR_BLUE); 
        Display_DrawText("> 3. Settings", 15, 90, 1, COLOR_WHITE);
    } else Display_DrawText("> 3. Settings", 15, 90, 1, 0x8410);
    
    // Tọa độ y=115 đảm bảo an toàn nằm dưới cùng màn hình
    Display_DrawText("Giu 'SW' 1.5s de Thoat", 10, 115, 1, COLOR_YELLOW);
}

void WindowManagerTask(void *pvParameters) {
    Serial.println("[OS] Window Manager khoi dong...");

    while (1) {
        if (request_full_redraw && system_state == 0) {
            force_redraw_menu = true;
            request_full_redraw = false;
        }
        if (system_state == 0) {
            if (force_redraw_menu) { drawMenu(); force_redraw_menu = false; }

            // [ĐÃ SỬA] Đọc Joystick qua API
            int joyY = Input_GetJoyY();
            if (joyY < 1000 && menu_index > 0) {
                menu_index--; drawMenu(); lastActivityTime = millis();
                vTaskDelay(200 / portTICK_PERIOD_MS); 
            } else if (joyY > 3000 && menu_index < 2) {
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
                } else if (menu_index == 2) {
                    // [MỚI] Gọi App Settings
                    Serial.println("[OS] Launching Settings App...");
                    system_state = 3; 
                    AppSettings_Start(); 
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