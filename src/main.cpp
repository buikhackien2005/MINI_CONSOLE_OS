#include <Arduino.h>
#include <SPI.h>
#include <esp_task_wdt.h>
#include "../../include/config.h"
#include "../../include/events.h"
#include "../../lib/InputManager/InputManager.h"

// 1. Queues
QueueHandle_t inputQueue;
QueueHandle_t renderQueue; 
QueueHandle_t mediaQueue; 

// [MỚI] 2. Biến lưu trữ Thẻ căn cước của các Task (Task Handles)
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t inputTaskHandle = NULL;
TaskHandle_t systemTaskHandle = NULL;
TaskHandle_t cliTaskHandle = NULL; // [MỚI] Thẻ căn cước cho Task Dòng lệnh

// 2. KHỞI TẠO LUỒNG SPI SỐ 2 (HSPI) CHO THẺ NHỚ
SPIClass hspi(HSPI);

extern void DisplayTask(void *pvParameters);
extern void InputTask(void *pvParameters);
extern void SystemTask(void *pvParameters);
extern void CliTask(void *pvParameters); // [MỚI] Khai báo hàm CLI

void setup() {
    Serial.begin(115200);
    esp_task_wdt_init(3, true); 

    // Bật điện cho bus HSPI (Thẻ SD)
    hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    InputManager::initPins();

    inputQueue = xQueueCreate(10, sizeof(InputEvent));
    renderQueue = xQueueCreate(5, sizeof(DisplayEvent)); 
    mediaQueue = xQueueCreate(10, sizeof(MediaEvent));

    // Xóa hết 4 dòng attachInterrupt cũ đi, thay bằng cấu hình nút SW của Joystick
    pinMode(JOY_SW_PIN, INPUT_PULLUP);

    // Tạo Tasks
    xTaskCreatePinnedToCore(DisplayTask, "DisplayTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(InputTask, "InputTask", 2048, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(SystemTask, "SystemTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(CliTask, "CliTask", 3072, NULL, 1, &cliTaskHandle, 0);
}


void loop() { vTaskDelete(NULL); }