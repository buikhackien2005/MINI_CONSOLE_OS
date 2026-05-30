#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include <TFT_eSPI.h> 
#include "../../include/config.h"
#include "../../lib/HAL_Display/DisplayDriver.h"

// [MỚI] Gọi Header của Game vào đây để lấy quyền khởi động Game
#include "../apps/app_pingpong.h"

extern TFT_eSPI tft; 
extern volatile int system_state;
extern volatile bool menu_selected;
extern volatile unsigned long lastActivityTime;

int menu_index = 0; 
bool force_redraw_menu = true;

// Hàm vẽ Giao diện Menu
void drawMenu() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN); tft.setTextSize(2);
    tft.setCursor(20, 10); tft.print("MINI OS");
    tft.drawFastHLine(0, 30, 160, TFT_GREEN);

    tft.setTextSize(1);
    if (menu_index == 0) { tft.fillRect(10, 45, 140, 20, TFT_BLUE); tft.setTextColor(TFT_WHITE); } 
    else tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(15, 50); tft.print("> 1. Game Ping Pong");

    if (menu_index == 1) { tft.fillRect(10, 75, 140, 20, TFT_BLUE); tft.setTextColor(TFT_WHITE); } 
    else tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(15, 80); tft.print("> 2. Music Player");
    
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(10, 110); tft.print("Giu 'SW' 1.5s de Thoat");
}

// ==========================================
// WINDOW MANAGER (Người gác cổng màn hình)
// ==========================================
void WindowManagerTask(void *pvParameters) {
    Serial.println("[OS] Window Manager khoi dong...");

    while (1) {
        if (system_state == 0) {
            // ĐANG Ở MENU
            if (force_redraw_menu) { drawMenu(); force_redraw_menu = false; }

            int joyY = analogRead(JOY_Y_PIN);
            if (joyY < 1000 && menu_index > 0) {
                menu_index--; drawMenu(); lastActivityTime = millis();
                vTaskDelay(200 / portTICK_PERIOD_MS); 
            } else if (joyY > 3000 && menu_index < 1) {
                menu_index++; drawMenu(); lastActivityTime = millis();
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }

            // NẾU BẤM CHỌN APP
            if (menu_selected) {
                menu_selected = false;
                if (menu_index == 0) {
                    Serial.println("[OS] Launching Ping Pong App...");
                    system_state = 1; // Chuyển trạng thái
                    AppPingPong_Start(); // LỆNH GỌI GAME CHẠY!
                } else if (menu_index == 1) {
                    tft.fillScreen(TFT_NAVY);
                    tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
                    tft.setCursor(25, 40); tft.print("MP3 PLAYER");
                    system_state = 2; 
                }
            }
        } 
        
        // Quản lý cờ vẽ lại Menu khi vừa thoát App
        static int last_system_state = 0;
        if (system_state != last_system_state) {
            if (system_state == 0) {
                force_redraw_menu = true; 
                menu_index = 0; 
            }
            last_system_state = system_state; 
        }

        // Nhường CPU cho các Task khác chạy (Bao gồm cả Game)
        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
}