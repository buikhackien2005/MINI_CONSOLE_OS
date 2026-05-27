#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// 1. CẤU HÌNH CỤM NÚT BẤM (D-PAD) - BÊN TRÁI
// ==========================================
#define BTN_UP_PIN    32
#define BTN_DOWN_PIN  33
#define BTN_LEFT_PIN  25
#define BTN_RIGHT_PIN 15
#define BTN_PRESSED_STATE 0 

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
#define I2S_BCLK 22
#define I2S_LRC  21
#define I2S_DIN  16

#endif