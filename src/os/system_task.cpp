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
extern volatile int system_state; // Để kiểm tra task nào đang chạy

// [MỚI] Mượn biến lưu độ sáng hiện tại từ OS để khôi phục sau khi ngủ dậy
extern int system_brightness; 
extern int system_sleep_timeout; // Thời gian sleep (giây, 0 = không bao giờ sleep)

// [MỚI] Mượn biến thời gian
extern int sys_hour, sys_minute, sys_second;

void SystemTask(void *pvParameters) {
    MediaEvent receivedEvent;
    extern volatile bool menu_selected; // Dùng để xóa cờ click nhầm khi thức dậy
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
            // [MỚI] TRÁI TIM ĐẾM NHỊP THỜI GIAN THỰC (RTC)
            // ==================================================
            static unsigned long last_time_tick = 0;
            if (millis() - last_time_tick >= 1000) {
                last_time_tick = millis();
                sys_second++;
                if (sys_second >= 60) {
                    sys_second = 0;
                    sys_minute++;
                    if (sys_minute >= 60) {
                        sys_minute = 0;
                        sys_hour++;
                        if (sys_hour >= 24) sys_hour = 0;
                    }
                    // Đồng hồ thay đổi mỗi phút -> Ra lệnh vẽ lại màn hình
                    request_full_redraw = true; 
                }
            }

            // ==================================================
            // 1. KIỂM TRA HOT-SWAP THẺ SD (Mỗi 2 giây)
            // Bỏ qua khi Music (state=2) hay Ping Pong (state=1) đang chạy
            // để tránh xung đột SPI bus với SD card
            // ==================================================
            static unsigned long last_sd_check = 0;
            if (millis() - last_sd_check > 2000) {
                last_sd_check = millis();

                if (system_state != 1 && system_state != 2) {
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
            }

            // ==================================================
            // 2. KIỂM TRA NGỦ ĐÔNG (SLEEP MODE)
            // ==================================================
            if (system_sleep_timeout > 0 &&
                millis() - lastActivityTime > (unsigned long)system_sleep_timeout * 1000UL) {
                Serial.println("\n[OS] He thong ranh roi. Di vao che do SLEEP...");

                Display_SetBrightness(0); // Tắt màn hình (cả 2 chế độ)

                if (system_state == 2) {
                    // =====================================================
                    // MUSIC SLEEP: CPU vẫn chạy → nhạc tiếp tục phát
                    // Polling SW pin, nhường CPU cho MusicTask qua vTaskDelay
                    // =====================================================
                    Serial.println("[OS] Music Sleep: Man hinh off, nhac tiep tuc phat...");

                    // Chờ người dùng nhấn SW (LOW = nhấn)
                    while (digitalRead(JOY_SW_PIN) != LOW) {
                        vTaskDelay(50 / portTICK_PERIOD_MS);
                    }
                    // Chờ người dùng thả SW
                    while (digitalRead(JOY_SW_PIN) == LOW) {
                        vTaskDelay(10 / portTICK_PERIOD_MS);
                    }
                } else {
                    // =====================================================
                    // LIGHT SLEEP: Đóng băng CPU, tiết kiệm điện
                    // =====================================================
                    gpio_wakeup_enable((gpio_num_t)JOY_SW_PIN, GPIO_INTR_LOW_LEVEL);
                    esp_sleep_enable_gpio_wakeup();
                    esp_light_sleep_start();
                }

                // --- THỨC DẬY (cả 2 chế độ đều đi qua đây) ---
                Serial.println("[OS] He thong da THUC DAY!");

                // Chờ InputTask xử lý xong button press rồi mới xóa cờ,
                // tránh SW dùng để thức dậy bị hiểu nhầm là "Enter"
                vTaskDelay(600 / portTICK_PERIOD_MS);
                menu_selected = false;

                Display_SetBrightness(system_brightness);
                lastActivityTime = millis();
            }
        }
    }
} 