#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_sleep.h"
#include "../../include/events.h"
#include "../../include/config.h"
#include "../../lib/HAL_Audio/AudioDriver.h"
#include "../../lib/HAL_Storage/StorageDriver.h"
#include "../../lib/HAL_Display/DisplayDriver.h" 

extern QueueHandle_t mediaQueue; 
extern volatile unsigned long lastActivityTime; 
extern volatile bool request_full_redraw; 

// [MỚI] Mượn biến lưu độ sáng hiện tại từ OS để khôi phục sau khi ngủ dậy
extern int system_brightness; 

void SystemTask(void *pvParameters) {
    MediaEvent receivedEvent;
    Serial.println("[OS] System Task (Core 0) dang chay ngam...");

    while (1) {
        // Lắng nghe Queue trong 1 giây
        if (xQueueReceive(mediaQueue, &receivedEvent, 1000 / portTICK_PERIOD_MS) == pdPASS) {
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
            // ==================================================
            // 1. KIỂM TRA HOT-SWAP THẺ SD (Mỗi 2 giây)
            // ==================================================
            static unsigned long last_sd_check = 0;
            if (millis() - last_sd_check > 2000) {
                last_sd_check = millis();
                
                int sd_status = Storage_CheckHotSwap(); // Chỉ gọi 1 lần duy nhất
                if (sd_status != 0) {
                    uint16_t box_color = (sd_status == 1) ? COLOR_GREEN : COLOR_BLUE; 
                    const char* msg = (sd_status == 1) ? " SD MOUNTED! " : " SD REMOVED! ";
                    Serial.println(msg);
                    
                    Display_DrawRect(20, 50, 120, 25, box_color);    
                    Display_DrawText(msg, 25, 58, 1, COLOR_WHITE);     

                    vTaskDelay(1500 / portTICK_PERIOD_MS);       
                    request_full_redraw = true; 
                }
            }

            // ==================================================
            // 2. KIỂM TRA NGỦ ĐÔNG (SLEEP MODE)
            // ==================================================
            if (millis() - lastActivityTime > SLEEP_TIMEOUT) {
                Serial.println("\n[OS] He thong ranh roi. Di vao che do SLEEP...");
                
                // [ĐÃ SỬA LỖI XUNG ĐỘT] Tắt màn hình bằng API chuẩn thay vì digitalWrite
                Display_SetBrightness(0); 
                
                // Cấu hình đánh thức bằng nút Joystick (SW)
                gpio_wakeup_enable((gpio_num_t)JOY_SW_PIN, GPIO_INTR_LOW_LEVEL);
                esp_sleep_enable_gpio_wakeup();
                
                // Đóng băng CPU (Ngủ đông)
                esp_light_sleep_start(); 

                // --- SAU KHI BẤM NÚT ĐỂ THỨC DẬY SẼ CHẠY TIẾP Ở ĐÂY ---
                Serial.println("[OS] He thong da THUC DAY!");
                
                // [ĐÃ SỬA LỖI XUNG ĐỘT] Bật lại đúng độ sáng mà người dùng đã cài
                Display_SetBrightness(system_brightness); 
                
                lastActivityTime = millis(); // Reset lại bộ đếm thời gian
            }
        }
    }
}