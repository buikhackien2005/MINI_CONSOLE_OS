#include <Arduino.h>
#include <SPI.h>
#include <esp_task_wdt.h>
#include "freertos/semphr.h"
#include "../../include/config.h"
#include "../../include/events.h"
#include "../../lib/InputManager/InputManager.h"

// ==========================================
// 1. KHAI BÁO BIẾN TOÀN CỤC (KHÔNG DÙNG EXTERN)
// ==========================================
QueueHandle_t inputQueue;
QueueHandle_t renderQueue; 
QueueHandle_t mediaQueue; 
SemaphoreHandle_t spiMutex; 

// ==========================================
// 2. KHAI BÁO CÁC HÀM TỪ FILE KHÁC BẰNG EXTERN
// ==========================================
extern void isrButtonA();
extern void isrButtonB();
extern void DisplayTask(void *pvParameters);
extern void InputTask(void *pvParameters);
extern void SystemTask(void *pvParameters);

void setup() {
    Serial.begin(115200);
    Serial.println("Khởi động Mini-Console OS...");

    // 1. Khởi tạo Watchdog Timer (Reset mạch sau 3s nếu bị treo)
    esp_task_wdt_init(3, true); 

    // 2. Khởi tạo Chìa khóa Mutex bảo vệ đường truyền SPI
    spiMutex = xSemaphoreCreateMutex();
    if (spiMutex == NULL) {
        Serial.println("[LỖI] Không thể tạo SPI Mutex! Sập hệ thống.");
        while (1);
    }

    // 3. Bật điện cho Bus SPI (Dùng chung cho TFT và SD Card)
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS_PIN);

    // 4. Khởi tạo chân nút bấm
    InputManager::initPins();

    // 5. Cấp phát bộ nhớ cho các Queue (Đường ống giao tiếp)
    inputQueue = xQueueCreate(10, sizeof(InputEvent));
    renderQueue = xQueueCreate(5, sizeof(DisplayEvent)); 
    mediaQueue = xQueueCreate(10, sizeof(MediaEvent));

    if (inputQueue == NULL || renderQueue == NULL || mediaQueue == NULL) {
        Serial.println("[LỖI] Tràn RAM, không thể tạo Queue!");
        while (1); 
    }

    // 6. Gắn Ngắt (ISR) cho các nút bấm vật lý
    attachInterrupt(digitalPinToInterrupt(BTN_A_PIN), isrButtonA, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_B_PIN), isrButtonB, FALLING); 

    // ==========================================
    // 7. KHỞI TẠO CÁC NHÂN CÔNG (TASKS)
    // ==========================================
    
    // Core 1: Lo đồ họa (Độ ưu tiên 1)
    xTaskCreatePinnedToCore(DisplayTask, "DisplayTask", 4096, NULL, 1, NULL, 1);

    // Core 1: Lo đọc phím (Độ ưu tiên cao nhất: 2)
    xTaskCreatePinnedToCore(InputTask, "InputTask", 2048, NULL, 2, NULL, 1);

    // Core 0: Lo âm thanh & Ghi thẻ nhớ (Độ ưu tiên 1)
    xTaskCreatePinnedToCore(SystemTask, "SystemTask", 4096, NULL, 1, NULL, 0);
}

void loop() {
    // Ép hàm loop() tự sát để nhường toàn bộ CPU cho FreeRTOS
    vTaskDelete(NULL); 
}