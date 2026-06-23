#include <Arduino.h>
#include <SPI.h>
#include <esp_task_wdt.h>
#include "esp_sleep.h"
#include "../include/config.h"
#include "../include/events.h"

// [MỚI] Gọi đúng đường dẫn Lớp trừu tượng HAL
#include "../lib/HAL_Input/InputDriver.h"
#include "../lib/HAL_Display/DisplayDriver.h"

// [MỚI] Sử dụng API của Tầng HAL
#include "../../lib/HAL_Audio/AudioDriver.h"
#include "../../lib/HAL_Storage/StorageDriver.h"

volatile unsigned long lastActivityTime = 0; 
int max_score = 7;             
float base_paddle_speed = 2.0; 
volatile int system_state = 0; 
volatile bool menu_selected = false;
volatile bool request_full_redraw = false; // [MỚI] Cờ báo hiệu cần khôi phục giao diện
int system_brightness = 100;   // [MỚI] Biến lưu độ sáng màn hình (10% - 100%)
int system_sleep_timeout = 30; // Thời gian chờ sleep (giây). 0 = không bao giờ sleep

// [MỚI] Biến lưu trữ thời gian thực (Mặc định khởi động là 12:00:00)
int sys_hour = 12;
int sys_minute = 0;
int sys_second = 0;

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

    // pinMode(TFT_BLK, OUTPUT);
    // digitalWrite(TFT_BLK, HIGH);
    lastActivityTime = millis();

    // [ĐÃ SỬA] Gọi hàm API chuẩn của HAL
    Display_Init();
    Input_Init();
    Audio_Init();     // [MỚI] Khởi tạo chân Loa
    
    // 1. LUÔN LUÔN KHỞI ĐỘNG ROOT FS TRƯỚC TIÊN
    Storage_Init_RootFS();
    
    // 2. Nạp ngay cấu hình từ Root FS vào bộ nhớ RAM
    Storage_LoadConfig();

    hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    Storage_Init_SD();       // [MỚI] Khởi tạo Thẻ nhớ SD

    inputQueue = xQueueCreate(10, sizeof(InputEvent));
    renderQueue = xQueueCreate(5, sizeof(DisplayEvent)); 
    mediaQueue = xQueueCreate(10, sizeof(MediaEvent));

    xTaskCreatePinnedToCore(WindowManagerTask, "WindowManager", 4096, NULL, 1, &windowManagerTaskHandle, 1);
    xTaskCreatePinnedToCore(InputTask, "InputTask", 2048, NULL, 2, &inputTaskHandle, 1);
    xTaskCreatePinnedToCore(SystemTask, "SystemTask", 4096, NULL, 1, &systemTaskHandle, 0);
    xTaskCreatePinnedToCore(CliTask, "CliTask", 4096, NULL, 1, &cliTaskHandle, 0);
}

void loop() { vTaskDelete(NULL); }