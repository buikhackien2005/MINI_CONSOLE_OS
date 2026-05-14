#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayManager {
public:
    static void init();
    static void clear();
    static void drawText(uint8_t x, uint8_t y, const char* text);
    static void update();
};

#endif