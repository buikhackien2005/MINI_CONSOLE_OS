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
extern int system_brightness;
extern volatile bool request_full_redraw;

static int settings_index = 0;
static bool force_redraw = true;

static void drawSettingsUI() {
    Display_FillScreen(COLOR_BG);
    Display_DrawText("SETTINGS", 35, 10, 2, COLOR_YELLOW);
    Display_DrawHLine(0, 30, 160, COLOR_YELLOW);

    // --- MỤC 1: ĐỘ SÁNG MÀN HÌNH ---
    uint16_t c1 = (settings_index == 0) ? COLOR_WHITE : 0x8410;
    Display_DrawText("1. Brightness", 10, 50, 1, c1);
    
    // Nếu đang chọn Độ sáng, hiển thị thêm < và > màu xanh lá
    if (settings_index == 0) {
        Display_DrawText("<", 95, 50, 1, COLOR_GREEN);
        Display_DrawInt(system_brightness, 110, 50, 1, c1, COLOR_BG);
        Display_DrawText(">", 135, 50, 1, COLOR_GREEN);
    } else {
        // Không chọn thì chỉ hiện số
        Display_DrawInt(system_brightness, 110, 50, 1, c1, COLOR_BG);
    }

    // --- MỤC 2: LƯU VÀ THOÁT ---
    uint16_t c2 = (settings_index == 1) ? COLOR_GREEN : 0x8410;
    Display_DrawText("2. SAVE & EXIT", 30, 80, 1, c2);
}

void SettingsTask(void *pvParameters) {
    force_redraw = true;
    menu_selected = false;
    settings_index = 0; 

    while (1) {
        // Lệnh giết Task từ OS khi user giữ nút HOME
        if (system_state != 3) { vTaskDelete(NULL); }

        // [MỚI] Tự chữa lành UI cho Settings
        if (request_full_redraw && system_state == 3) {
            force_redraw = true;
            request_full_redraw = false;
        }

        if (force_redraw) {
            drawSettingsUI();
            force_redraw = false;
        }

        // 1. Quản lý cuộn lên/xuống (Joystick Y)
        int joyY = Input_GetJoyY();
        if (joyY < 1000 && settings_index > 0) {
            settings_index--; force_redraw = true; lastActivityTime = millis();
            vTaskDelay(200 / portTICK_PERIOD_MS); 
        } else if (joyY > 3000 && settings_index < 1) {
            settings_index++; force_redraw = true; lastActivityTime = millis();
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }

        // 2. Quản lý chỉnh Độ sáng (Joystick X)
        if (settings_index == 0) {
            int joyX = Input_GetJoyX();
            if (joyX < 1000) { // Gạt trái
                system_brightness -= 10;
                if (system_brightness < 10) system_brightness = 10;
                Display_SetBrightness(system_brightness);
                force_redraw = true; lastActivityTime = millis();
                vTaskDelay(150 / portTICK_PERIOD_MS); 
            } else if (joyX > 3000) { // Gạt phải
                system_brightness += 10;
                if (system_brightness > 100) system_brightness = 100;
                Display_SetBrightness(system_brightness);
                force_redraw = true; lastActivityTime = millis();
                vTaskDelay(150 / portTICK_PERIOD_MS);
            }
        }

        // 3. Quản lý Nhấn nút SW
        if (menu_selected) {
            menu_selected = false;
            lastActivityTime = millis();
            
            if (settings_index == 1) { // SAVE & EXIT
                Storage_SaveConfig();
                
                Display_FillScreen(COLOR_BG);
                Display_DrawText("SAVED!", 50, 60, 2, COLOR_GREEN);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                
                system_state = 0; // Trả về màn hình Menu chính
            }
        }

        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
}

void AppSettings_Start() {
    xTaskCreatePinnedToCore(SettingsTask, "Settings", 3072, NULL, 1, NULL, 1);
}