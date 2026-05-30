#include <Arduino.h>
#include <SPI.h>
#include <esp_task_wdt.h>
#include "esp_sleep.h"
#include "../../include/config.h"
#include "../../include/events.h"
#include "../../lib/HAL_Input/InputDriver.h"
#include "../../lib/HAL_Display/DisplayDriver.h"

// ==========================================
// 1. BIẾN TOÀN CỤC CỦA HỆ ĐIỀU HÀNH
// ==========================================
volatile unsigned long lastActivityTime = 0; 

// Thông số Game mặc định (Dùng khi không có thẻ nhớ)
int max_score = 7;             
float base_paddle_speed = 2.0; 

// Biến điều khiển Trạng thái (0 = Menu, 1 = Game, 2 = Music)
volatile int system_state = 0; 
volatile bool menu_selected = false; 

// ==========================================
// 2. QUEUE & THẺ CĂN CƯỚC TASK
// ==========================================
QueueHandle_t inputQueue;
QueueHandle_t renderQueue; 
QueueHandle_t mediaQueue; 

TaskHandle_t windowManagerTaskHandle = NULL;
TaskHandle_t inputTaskHandle = NULL;
TaskHandle_t systemTaskHandle = NULL;
TaskHandle_t cliTaskHandle = NULL; 

SPIClass hspi(HSPI);

extern void WindowManagerTask(void *pvParameters);
extern void InputTask(void *pvParameters);
extern void SystemTask(void *pvParameters);
extern void CliTask(void *pvParameters); 

void setup() {
    Serial.begin(115200);
    esp_task_wdt_init(3, true); 

    // Bật đèn nền và khởi động đồng hồ bấm giờ Sleep Mode
    pinMode(TFT_BLK, OUTPUT);
    digitalWrite(TFT_BLK, HIGH);
    lastActivityTime = millis();

    // Khởi tạo Màn hình đầu tiên để chiếm bus SPI
    DisplayDriver::init();

    // Khởi tạo điện cho thẻ nhớ (Dù chưa cắm thẻ vẫn phải gọi để chốt cấu hình SPI)
    hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    InputDriver::initPins();

    inputQueue = xQueueCreate(10, sizeof(InputEvent));
    renderQueue = xQueueCreate(5, sizeof(DisplayEvent)); 
    mediaQueue = xQueueCreate(10, sizeof(MediaEvent));

    pinMode(JOY_SW_PIN, INPUT_PULLUP);

    // Kích hoạt các Task
    xTaskCreatePinnedToCore(WindowManagerTask, "WindowManagerTask", 4096, NULL, 1, &windowManagerTaskHandle, 1);
    xTaskCreatePinnedToCore(InputTask, "InputTask", 2048, NULL, 2, &inputTaskHandle, 1);
    xTaskCreatePinnedToCore(SystemTask, "SystemTask", 4096, NULL, 1, &systemTaskHandle, 0);
    xTaskCreatePinnedToCore(CliTask, "CliTask", 3072, NULL, 1, &cliTaskHandle, 0);
}

void loop() { 
    vTaskDelete(NULL); 
}