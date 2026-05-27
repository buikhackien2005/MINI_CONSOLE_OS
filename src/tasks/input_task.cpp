#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../../include/events.h"

extern QueueHandle_t inputQueue;
extern QueueHandle_t renderQueue;
extern QueueHandle_t mediaQueue;

void InputTask(void *pvParameters) {
    Serial.println("[Task] InputTask (Core 1) đang chạy...");
    InputEvent receivedEvent;

    while (1) {
        // Trả lại lệnh portMAX_DELAY: Task sẽ ngủ hoàn toàn 0% CPU cho đến khi có Ngắt
        if (xQueueReceive(inputQueue, &receivedEvent, portMAX_DELAY) == pdPASS) {
            
            if (receivedEvent.buttonID == 1) { 
                Serial.println("-> Nút UP: Vẽ ảnh Lopaka!");
                DisplayEvent drawCmd = {2, 0, 0, ""};
                xQueueSend(renderQueue, &drawCmd, portMAX_DELAY);
            }
            else if (receivedEvent.buttonID == 2) { 
                Serial.println("-> Nút DOWN: Xóa màn hình!");
                DisplayEvent clearCmd = {0, 0, 0, ""};
                xQueueSend(renderQueue, &clearCmd, portMAX_DELAY);
            }
            else if (receivedEvent.buttonID == 3) { 
                Serial.println("-> Nút LEFT: Phát tiếng Bíp!");
                MediaEvent beepCmd;
                beepCmd.cmdType = 1; 
                xQueueSend(mediaQueue, &beepCmd, portMAX_DELAY);
            }
            else if (receivedEvent.buttonID == 4) { 
                Serial.println("-> Nút RIGHT: Ghi Log SD!");
                MediaEvent logCmd;
                logCmd.cmdType = 2;
                strcpy(logCmd.logData, "User bam nut RIGHT"); 
                xQueueSend(mediaQueue, &logCmd, portMAX_DELAY);
            }

        }
    }
}