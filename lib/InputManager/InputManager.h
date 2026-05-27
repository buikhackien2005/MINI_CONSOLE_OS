#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H
#include <Arduino.h>

class InputManager {
public:
    static void initPins();
};

// Khai báo 4 hàm Ngắt cho 4 nút
extern void isrButtonUp();
extern void isrButtonDown();
extern void isrButtonLeft();
extern void isrButtonRight();

#endif