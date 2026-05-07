#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h> // Gọi thư viện lõi nếu dùng PlatformIO Arduino framework

class InputManager {
public:
    static void initPins(); // Khai báo hàm static để gọi thẳng không cần khởi tạo object
};

#endif