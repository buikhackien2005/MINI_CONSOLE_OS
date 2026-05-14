#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>

struct InputEvent {
    uint8_t buttonID;
    bool isPressed;
};

struct DisplayEvent {
    uint8_t cmdType;
    uint8_t x;
    uint8_t y;
    char text[20];
};

// [MỚI] Struct lệnh điều khiển Media (Âm thanh & Lưu trữ)
struct MediaEvent {
    uint8_t cmdType; // 1: Phát tiếng Bíp (Nháy đèn), 2: Ghi Log
    char logData[30]; // Nội dung cần ghi vào thẻ nhớ ảo (SPIFFS)
};

#endif