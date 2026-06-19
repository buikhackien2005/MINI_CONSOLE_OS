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

extern int sys_hour, sys_minute;
extern bool is_sd_mounted;

int menu_index = 0; 
bool force_redraw_menu = true;

void drawMenu() {
    Display_FillScreen(COLOR_BLACK);
    
    // 1. Vẽ chữ "MINI OS" ở chính giữa, trên cùng (Thoáng đãng hoàn toàn)
    Display_DrawText("MINI OS", 38, 5, 2, COLOR_GREEN); 

    // 2. Vẽ danh sách Menu
    if (menu_index == 0) { 
        Display_DrawRect(10, 35, 140, 18, COLOR_BLUE); 
        Display_DrawText("> 1. Game Ping Pong", 15, 40, 1, COLOR_WHITE);
    } else Display_DrawText("> 1. Game Ping Pong", 15, 40, 1, 0x8410); 

    if (menu_index == 1) { 
        Display_DrawRect(10, 55, 140, 18, COLOR_BLUE); 
        Display_DrawText("> 2. Music Player", 15, 60, 1, COLOR_WHITE);
    } else Display_DrawText("> 2. Music Player", 15, 60, 1, 0x8410);
    
    if (menu_index == 2) { 
        Display_DrawRect(10, 75, 140, 18, COLOR_BLUE); 
        Display_DrawText("> 3. Settings", 15, 80, 1, COLOR_WHITE);
    } else Display_DrawText("> 3. Settings", 15, 80, 1, 0x8410);
    
    // 3. Đẩy dòng chữ hướng dẫn lên Y = 100 để nhường chỗ cho Đáy
    Display_DrawText("Giu 'SW' de Thoat", 25, 100, 1, COLOR_YELLOW);

    // 4. Vẽ thanh Trạng thái (SD & Giờ) đè lên dưới cùng
    Display_DrawTaskbar(sys_hour, sys_minute, is_sd_mounted);
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