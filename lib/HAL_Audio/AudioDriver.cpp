#include <Arduino.h>
#include <driver/i2s.h>
#include "../../include/config.h"
#include "AudioDriver.h"

#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000

void Audio_Init() {
    Serial.println("[HAL] Dang khoi tao Amply I2S...");
    
    // 1. Cấu hình thông số kỹ thuật cho I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Phát mono 1 kênh
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    // 2. Map các chân vật lý từ config.h
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRC,
        .data_out_num = I2S_DIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    // 3. Cài đặt driver và xóa nhiễu DMA
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_zero_dma_buffer(I2S_PORT); 
}

void Audio_PlayBeep() {
    // Tạo một sóng vuông (Square wave) đơn giản ở tần số 1000Hz làm tiếng Bíp
    size_t bytes_written;
    int16_t sample = 0;
    int frequency = 1000; 
    int half_period = SAMPLE_RATE / frequency / 2;

    // Phát âm thanh trong 0.05 giây (Chạm bóng phát tiếng tạch ngắn)
    for (int i = 0; i < SAMPLE_RATE / 20; i++) { 
        if ((i / half_period) % 2 == 0) {
            sample = 15000;  // Biên độ dương (1500 là âm lượng vừa phải, tối đa là 32767)
        } else {
            sample = -1500; // Biên độ âm
        }
        i2s_write(I2S_PORT, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    }
    
    // Đẩy tín hiệu rỗng để loa lập tức im lặng, tránh nhiễu rè xì xì
    i2s_zero_dma_buffer(I2S_PORT); 
}