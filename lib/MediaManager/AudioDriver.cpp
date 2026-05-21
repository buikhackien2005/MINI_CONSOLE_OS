#include "AudioDriver.h"
#include "../../include/config.h"
#include <driver/i2s.h>

void AudioDriver::init() {
    // Cấu hình giao thức I2S như trong file test
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRC,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    i2s_set_pin(I2S_NUM_0, &pin_config);
    
    Serial.println("[HAL: Audio] Đã khởi tạo Loa I2S (MAX98357A).");
}

void AudioDriver::playBeep() {
    // Tạo tiếng bíp đơn giản (0.1 giây)
    size_t bytes_written;
    int16_t sample_val = 0;
    
    for(int i = 0; i < 4410; i++) { // 4410 samples = ~0.1 giây ở sample rate 44100
        sample_val = (i % 100 < 50) ? 2000 : -2000; 
        i2s_write(I2S_NUM_0, &sample_val, sizeof(sample_val), &bytes_written, portMAX_DELAY);
    }
}