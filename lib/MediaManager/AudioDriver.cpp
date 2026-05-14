// --- AudioDriver.cpp ---
#include "AudioDriver.h"
#include "../../include/config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void AudioDriver::init() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); // Tắt đèn/còi
    Serial.println("[HAL: Audio] Đã khởi tạo hệ thống âm thanh (Giả lập LED).");
}

void AudioDriver::playBeep() {
    digitalWrite(BUZZER_PIN, HIGH); // Sáng đèn
    vTaskDelay(100 / portTICK_PERIOD_MS); // Giữ 100ms
    digitalWrite(BUZZER_PIN, LOW);  // Tắt đèn
}