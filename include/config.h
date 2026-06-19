#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// 1. CẤU HÌNH MODULE JOYSTICK ANALOG
// ==========================================
#define JOY_Y_PIN 32  // Trục Y (Lên/Xuống)
#define JOY_X_PIN 35  // [MỚI] Chân VRx của Joystick
#define JOY_SW_PIN 33  // Nút nhấn tích hợp

// ==========================================
// 2. BUS VSPI CHO MÀN HÌNH TFT - BÊN PHẢI
// ==========================================
#define TFT_MOSI 23
#define TFT_SCK  18
#define TFT_CS   5
#define TFT_DC   19
#define TFT_RST  17
#define TFT_WIDTH 128
#define TFT_HEIGHT 160

// ==========================================
// 3. BUS HSPI CHO THẺ NHỚ SD - BÊN TRÁI
// ==========================================
#define SD_MOSI 13
#define SD_MISO 4
#define SD_SCK  14
#define SD_CS   27

// ==========================================
// 4. CẤU HÌNH ÂM THANH I2S - BÊN PHẢI
// ==========================================
#define I2S_BCLK 22  // Bit Clock
#define I2S_LRC  21  // Left/Right Clock (Word Select)
#define I2S_DIN  16  // Data Input (Nối với DOUT của ESP32)

// ==========================================
// 5. CẤU HÌNH QUẢN LÝ NĂNG LƯỢNG (SLEEP MODE)
// ==========================================
#define TFT_BLK 26           // Chân điều khiển đèn nền màn hình (Cắm vào D26)
#define SLEEP_TIMEOUT 30000  // Thời gian chờ để ngủ (30.000 ms = 30 giây)

#endif