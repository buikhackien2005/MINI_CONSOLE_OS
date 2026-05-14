#include "InputManager.h"
#include "../../include/config.h"

void InputManager::initPins() {
    pinMode(BTN_A_PIN, INPUT_PULLUP);
    pinMode(BTN_B_PIN, INPUT_PULLUP);
}