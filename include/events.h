#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>

// Struct gói tin sự kiện
struct InputEvent {
    uint8_t buttonID;   // Ví dụ: 1 là nút A, 2 là nút B
    bool isPressed;     // true: đang nhấn, false: thả ra
};

#endif