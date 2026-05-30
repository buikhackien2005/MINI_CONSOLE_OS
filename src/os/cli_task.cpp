#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "../../include/events.h"

extern QueueHandle_t mediaQueue; 
extern TaskHandle_t windowManagerTaskHandle;
extern TaskHandle_t inputTaskHandle;
extern TaskHandle_t systemTaskHandle;
extern TaskHandle_t cliTaskHandle;

void CliTask(void *pvParameters) {
    // [FIX 2] Bắt CLI ngủ 2 giây lúc mới khởi động.
    // Việc này giúp nhường đường cho các Task khác in log xong xuôi,
    // và cho bạn đủ thời gian bật cửa sổ Serial Monitor lên.
    vTaskDelay(2000 / portTICK_PERIOD_MS); 
    
    Serial.println("\n\n=======================================");
    Serial.println("   [HỆ THỐNG] MINI-CONSOLE OS DA SAN SANG");
    Serial.println("   Go 'help' de xem danh sach lenh.");
    Serial.println("=======================================\n");
    
    // In dấu nhắc lệnh ban đầu (Đảm bảo không bị trôi)
    Serial.print("OS_Shell> "); 

    while (1) {
        if (Serial.available() > 0) {
            String cmd = Serial.readStringUntil('\n');
            cmd.trim(); 
            
            if (cmd.length() > 0) {
                // [FIX 1] MANUAL ECHO: Bắt ESP32 in thẳng chữ bạn vừa gõ ra màn hình!
                Serial.println(cmd); 
                
                // --- BỘ XỬ LÝ LỆNH ---
                if (cmd == "help") {
                    Serial.println("Lenh: help, ping, mem, beep, reboot");
                } 
                else if (cmd == "ping") {
                    Serial.println("pong!");
                } 
                else if (cmd == "mem") {
                    Serial.printf("RAM Trong: %d bytes\n", xPortGetFreeHeapSize());
                    if (windowManagerTaskHandle) Serial.printf("WindowManagerTask du: %d words\n", uxTaskGetStackHighWaterMark(windowManagerTaskHandle));
                    if (inputTaskHandle)   Serial.printf("InputTask du:   %d words\n", uxTaskGetStackHighWaterMark(inputTaskHandle));
                    if (systemTaskHandle)  Serial.printf("SystemTask du:  %d words\n", uxTaskGetStackHighWaterMark(systemTaskHandle));
                    if (cliTaskHandle)     Serial.printf("CliTask du:     %d words\n", uxTaskGetStackHighWaterMark(cliTaskHandle));
                } 
                else if (cmd == "beep") {
                    MediaEvent beepCmd; beepCmd.cmdType = 1; xQueueSend(mediaQueue, &beepCmd, 0); 
                    Serial.println("Da phat tieng Bip!");
                } 
                else if (cmd == "reboot") {
                    Serial.println("Rebooting..."); vTaskDelay(500); ESP.restart(); 
                } 
                else {
                    Serial.println("Lenh khong hop le!");
                }
                
                // In lại dấu nhắc lệnh chờ bạn gõ tiếp
                Serial.print("\nOS_Shell> "); 
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}