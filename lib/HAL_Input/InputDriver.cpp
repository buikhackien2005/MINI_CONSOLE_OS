#include <Arduino.h>
#include "../../include/config.h"
#include "InputDriver.h"

void Input_Init() {
    pinMode(JOY_SW_PIN, INPUT_PULLUP);
}

int Input_GetJoyY() {
    // Ẩn giấu hàm analogRead của ESP32 bên trong này
    return analogRead(JOY_Y_PIN);
}

int Input_GetJoyX() {
    return analogRead(JOY_X_PIN);
}

bool Input_IsHomePressed() {
    return digitalRead(JOY_SW_PIN) == LOW;
}