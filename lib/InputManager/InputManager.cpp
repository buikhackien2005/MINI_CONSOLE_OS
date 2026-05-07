#include "InputManager.h"
#include "../../include/config.h"

void InputManager::initPins() {
    // Kích hoạt điện trở kéo lên (Pull-up) bên trong ESP32
    pinMode(BTN_A_PIN, INPUT_PULLUP);
    pinMode(BTN_B_PIN, INPUT_PULLUP);
}