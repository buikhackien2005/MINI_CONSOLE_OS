#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// 1. CẤU HÌNH NÚT BẤM (INPUT)
// ==========================================
#define BTN_A_PIN 15
#define BTN_B_PIN 5      // Đã cập nhật theo mạch của bạn
#define BTN_PRESSED_STATE 0 

// ==========================================
// 2. CẤU HÌNH BUS SPI (DÙNG CHUNG CHO TFT & SD)
// ==========================================
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK  18

// ==========================================
// 3. CẤU HÌNH MÀN HÌNH TFT 1.77 INCH (DISPLAY)
// ==========================================
// (Lưu ý: Các chân SPI của màn hình đã được cấu hình trong platformio.ini)
#define TFT_WIDTH 128
#define TFT_HEIGHT 160

// ==========================================
// 4. CẤU HÌNH THẺ NHỚ MICRO SD (STORAGE)
// ==========================================
#define SD_CS_PIN 4

// ==========================================
// 5. CẤU HÌNH ÂM THANH I2S MAX98357A (AUDIO)
// ==========================================
#define I2S_DOUT 25
#define I2S_BCLK 22
#define I2S_LRC  21

#endif