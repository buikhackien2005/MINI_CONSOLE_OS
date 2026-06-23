#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../include/config.h"
#include "../../lib/HAL_Display/DisplayDriver.h"
#include "../../lib/HAL_Input/InputDriver.h"
#include "../../lib/HAL_Storage/StorageDriver.h"

extern volatile int system_state;
extern volatile bool menu_selected;
extern volatile unsigned long lastActivityTime;
extern volatile bool request_full_redraw; 

extern int system_brightness;
extern int sys_hour;
extern int sys_minute;
extern int sys_second;
extern int system_sleep_timeout; // Thời gian sleep (giây, 0 = không bao giờ sleep)

// [MỚI] Quản lý Trang (0: Màn hình chính, 1: Màn hình chỉnh giờ)
static int settings_page = 0; 
static int settings_index = 0;
static bool force_redraw = true;

static void drawSettingsUI() {
    Display_FillScreen(COLOR_BG);
    
    // ==========================================
    // TRANG 0: MENU CHÍNH CỦA SETTINGS
    // ==========================================
    if (settings_page == 0) {
        Display_DrawText("SETTINGS", 35, 10, 2, COLOR_YELLOW);
        Display_DrawHLine(0, 30, 160, COLOR_YELLOW);

        // 1. Độ sáng
        uint16_t c1 = (settings_index == 0) ? COLOR_WHITE : 0x8410;
        Display_DrawText("1. Brightness", 5, 40, 1, c1);
        if (settings_index == 0) {
            Display_DrawText("<", 95, 40, 1, COLOR_GREEN);
            Display_DrawInt(system_brightness, 108, 40, 1, c1, COLOR_BG);
            Display_DrawText(">", 126, 40, 1, COLOR_GREEN);
        } else {
            Display_DrawInt(system_brightness, 108, 40, 1, c1, COLOR_BG);
        }

        // 2. Sleep Timeout [MỚI]
        uint16_t c2 = (settings_index == 1) ? COLOR_WHITE : 0x8410;
        Display_DrawText("2. Sleep", 5, 57, 1, c2);
        char to_str[6];
        if (system_sleep_timeout == 0) strcpy(to_str, "OFF");
        else sprintf(to_str, "%ds", system_sleep_timeout);
        if (settings_index == 1) {
            Display_DrawText("<", 72, 57, 1, COLOR_GREEN);
            Display_DrawText(to_str, 84, 57, 1, c2);
            Display_DrawText(">", 114, 57, 1, COLOR_GREEN);
        } else {
            Display_DrawText(to_str, 84, 57, 1, c2);
        }

        // 3. Chuyển sang Cài đặt Thời gian
        uint16_t c3 = (settings_index == 2) ? COLOR_WHITE : 0x8410;
        Display_DrawText("3. Time Settings", 5, 74, 1, c3);

        // 4. Lưu & Thoát
        uint16_t c4 = (settings_index == 3) ? COLOR_GREEN : 0x8410;
        Display_DrawText("4. SAVE & EXIT", 20, 100, 1, c4);
    } 
    // ==========================================
    // TRANG 1: MÀN HÌNH CHỈNH THỜI GIAN
    // ==========================================
    else if (settings_page == 1) {
        Display_DrawText("TIME SETTING", 15, 10, 2, COLOR_YELLOW);
        Display_DrawHLine(0, 30, 160, COLOR_YELLOW);

        // 1. Chỉnh Giờ
        uint16_t c1 = (settings_index == 0) ? COLOR_WHITE : 0x8410;
        Display_DrawText("1. Hour", 5, 45, 1, c1);
        if (settings_index == 0) {
            Display_DrawText("<", 80, 45, 1, COLOR_GREEN);
            char h_str[5]; sprintf(h_str, "%02d", sys_hour);
            Display_DrawText(h_str, 95, 45, 1, c1);
            Display_DrawText(">", 120, 45, 1, COLOR_GREEN);
        } else {
            char h_str[5]; sprintf(h_str, "%02d", sys_hour);
            Display_DrawText(h_str, 95, 45, 1, c1);
        }

        // 2. Chỉnh Phút
        uint16_t c2 = (settings_index == 1) ? COLOR_WHITE : 0x8410;
        Display_DrawText("2. Minute", 5, 65, 1, c2);
        if (settings_index == 1) {
            Display_DrawText("<", 80, 65, 1, COLOR_GREEN);
            char m_str[5]; sprintf(m_str, "%02d", sys_minute);
            Display_DrawText(m_str, 95, 65, 1, c2);
            Display_DrawText(">", 120, 65, 1, COLOR_GREEN);
        } else {
            char m_str[5]; sprintf(m_str, "%02d", sys_minute);
            Display_DrawText(m_str, 95, 65, 1, c2);
        }

        // 3. Nút Quay lại
        uint16_t c3 = (settings_index == 2) ? COLOR_RED : 0x8410;
        Display_DrawText("3. BACK", 50, 95, 1, c3);
    }
}

void SettingsTask(void *pvParameters) {
    force_redraw = true;
    menu_selected = false;
    settings_page = 0; // Luôn bắt đầu ở Trang 0
    settings_index = 0; 

    while (1) {
        if (system_state != 3) { vTaskDelete(NULL); }

        if (request_full_redraw && system_state == 3) {
            force_redraw = true;
            request_full_redraw = false;
        }

        if (force_redraw) {
            drawSettingsUI();
            force_redraw = false;
        }

        // 1. Quản lý Joystick Lên/Xuống
        int joyY = Input_GetJoyY();
        if (joyY < 1000 && settings_index > 0) {
            settings_index--; force_redraw = true; lastActivityTime = millis();
            vTaskDelay(200 / portTICK_PERIOD_MS); 
        } else if (joyY > 3000 && settings_index < 3) { 
            settings_index++; force_redraw = true; lastActivityTime = millis();
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }

        // 2. Quản lý Joystick Trái/Phải
        int joyX = Input_GetJoyX();
        if (joyX < 1000 || joyX > 3000) {
            bool is_left = (joyX < 1000);
            
            if (settings_page == 0 && settings_index == 0) { // Đang ở Trang 0, Cột Độ sáng
                if (is_left) system_brightness -= 10;
                else system_brightness += 10;
                
                if (system_brightness < 10) system_brightness = 10;
                if (system_brightness > 100) system_brightness = 100;
                Display_SetBrightness(system_brightness);
                force_redraw = true; lastActivityTime = millis();
                vTaskDelay(150 / portTICK_PERIOD_MS); 
            } 
            else if (settings_page == 0 && settings_index == 1) { // Sleep Timeout [MỚI]
                if (is_left) system_sleep_timeout -= 15;
                else         system_sleep_timeout += 15;
                if (system_sleep_timeout < 0)   system_sleep_timeout = 0;   // 0 = OFF
                if (system_sleep_timeout > 120) system_sleep_timeout = 120;
                force_redraw = true; lastActivityTime = millis();
                vTaskDelay(150 / portTICK_PERIOD_MS);
            }
            else if (settings_page == 1) { // Đang ở Trang 1
                if (settings_index == 0) { // Chỉnh Giờ
                    if (is_left) sys_hour--; else sys_hour++;
                    if (sys_hour < 0) sys_hour = 23;
                    if (sys_hour > 23) sys_hour = 0;
                }
                else if (settings_index == 1) { // Chỉnh Phút
                    if (is_left) sys_minute--; else sys_minute++;
                    if (sys_minute < 0) sys_minute = 59;
                    if (sys_minute > 59) sys_minute = 0;
                    sys_second = 0; 
                }
                force_redraw = true; lastActivityTime = millis();
                vTaskDelay(150 / portTICK_PERIOD_MS); 
            }
        }

        // 3. Quản lý Click Nút SW
        if (menu_selected) {
            menu_selected = false;
            lastActivityTime = millis();
            
            if (settings_page == 0) {
                if (settings_index == 2) { // Click "Time Settings"
                    settings_page = 1;     // Nhảy sang Trang 1
                    settings_index = 0;    // Reset con trỏ
                    force_redraw = true;
                } else if (settings_index == 3) { // Click "SAVE & EXIT"
                    Storage_SaveConfig(); 
                    Display_FillScreen(COLOR_BG);
                    Display_DrawText("SAVED!", 50, 60, 2, COLOR_GREEN);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    system_state = 0; 
                }
            } 
            else if (settings_page == 1) {
                if (settings_index == 2) { // Click "BACK"
                    settings_page = 0;     // Lùi về Trang 0
                    settings_index = 1;    // Đặt con trỏ vào đúng chỗ "Time Settings"
                    force_redraw = true;
                }
            }
        }

        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
}

void AppSettings_Start() {
    xTaskCreatePinnedToCore(SettingsTask, "Settings", 3072, NULL, 1, NULL, 1);
}