#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h> // Sử dụng thư viện mới

class DisplayManager {
public:
    static void init();
    static void clear();
    static void drawText(uint8_t x, uint8_t y, const char* text);
    static void drawImage();
};

#endif