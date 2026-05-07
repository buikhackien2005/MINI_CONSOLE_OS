#ifndef CONFIG_H
#define CONFIG_H

// Khai báo chân GPIO kết nối với nút bấm (Giả sử dùng chân 12 và 13 trên ESP32)
#define BTN_A_PIN 12
#define BTN_B_PIN 13

// Mức logic khi nút được nhấn (Dùng INPUT_PULLUP thì khi nhấn sẽ là LOW - mức 0)
#define BTN_PRESSED_STATE 0 

#endif