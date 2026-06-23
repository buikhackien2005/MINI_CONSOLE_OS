#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// 1. CẤU HÌNH JOYSTICK (ĐẦU VÀO)
// ==========================================
#define JOY_Y_PIN  32   // Trục Y (Lên/Xuống)
#define JOY_X_PIN  35   // Trục X (Trái/Phải)
#define JOY_SW_PIN 33   // Nút nhấn Joystick

// ==========================================
// 2. BUS VSPI CHO MÀN HÌNH TFT (MẶT PHẢI)
// ==========================================
#define TFT_MOSI   23   // Chân SDA trên mạch TFT
#define TFT_SCK    18   // Chân SCK trên mạch TFT
#define TFT_CS     5    // Chân CS trên mạch TFT
#define TFT_DC     19   // Chân RS trên mạch TFT
#define TFT_RST    17   // Chân RES trên mạch TFT
#define TFT_WIDTH  128
#define TFT_HEIGHT 160

// ==========================================
// 3. BUS HSPI CHO THẺ NHỚ SD (MẶT TRÁI)
// ==========================================
#define SD_MOSI    13   // Chân MOSI trên module SD
#define SD_MISO    4    // Chân MISO trên module SD
#define SD_SCK     14   // Chân SCK trên module SD
#define SD_CS      27   // Chân CS trên module SD

// ==========================================
// 4. CẤU HÌNH ÂM THANH I2S (MẠCH MAX98357A)
// ==========================================
#define I2S_BCLK   22   // Chân BCLK
#define I2S_LRC    21   // Chân LRC
#define I2S_DIN    25   // Chân DIN (Đã chuyển sang chân 25 an toàn)

// ==========================================
// 5. CẤU HÌNH QUẢN LÝ NĂNG LƯỢNG & ĐÈN NỀN
// ==========================================
#define TFT_BLK    26   // Chân LEDA (Điều khiển bằng PWM)
#define SLEEP_TIMEOUT 30000 // Thời gian chờ ngủ đông (30 giây)

// ==========================================
// 6. BẢNG MÀU HỆ THỐNG (CHUẨN 16-BIT RGB565)
// ==========================================
#define COLOR_BLACK  0x0000
#define COLOR_WHITE  0xFFFF
#define COLOR_RED    0xF800
#define COLOR_GREEN  0x07E0
#define COLOR_BLUE   0x001F
#define COLOR_YELLOW 0xFFE0
#define COLOR_BG     0x0000 // Màu nền mặc định

#endif