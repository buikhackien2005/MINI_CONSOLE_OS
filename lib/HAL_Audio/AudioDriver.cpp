#include <Arduino.h>
#include "../../include/config.h"
#include "AudioDriver.h"
#include <SD.h>
#include "Audio.h" // Thư viện giải mã MP3

// [QUAN TRỌNG NHẤT] Khởi tạo đối tượng 'audio' TOÀN CỤC ở đây để các hàm bên dưới có thể dùng!
Audio audio; 

// Cờ báo hiệu bài hát vừa kết thúc (được set bởi callback audio_eof_mp3)
static volatile bool s_track_ended = false; 

void Audio_Init() {
    // Tạm thời để trống.                                                 
    // Chúng ta không dùng driver i2s gốc của ESP32 nữa để tránh tranh chấp phần cứng với thư viện MP3.
}                                                                                                                                                                   

void Audio_PlayBeep() {
    // Tạm thời để trống. (Sẽ dùng Amply I2S phát file tiếng Beep .mp3 sau nếu cần)
}

// ==========================================
// [MỚI] TRÌNH ĐIỀU KHIỂN MP3 QUA I2S
// ==========================================
void Audio_InitI2S() {
    Serial.println("[HAL] Khoi tao Amply I2S bang ESP32-audioI2S...");
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DIN);
    audio.setVolume(15); // Âm lượng mặc định khi khởi động (app sẽ override ngay sau)
    s_track_ended = false; // Reset cờ khi khởi tạo
}

void Audio_PlayMusic(const char* path) {
    audio.connecttoFS(SD, path);
}

void Audio_PauseResume() {
    audio.pauseResume();
}

void Audio_Stop() {
    audio.stopSong();
}

void Audio_Loop() {
    audio.loop(); // Liên tục nạp dữ liệu từ SD vào bộ đệm I2S
}

bool Audio_IsPlaying() {
    return audio.isRunning();
}

void Audio_SetVolume(int vol) {
    if (vol < 0)  vol = 0;
    if (vol > 20) vol = 20;
    audio.setVolume(vol);
}

bool Audio_IsTrackEnded() {
    return s_track_ended;
}

void Audio_ClearTrackEnded() {
    s_track_ended = false;
}

// ==========================================
// CÁC HÀM LẮNG NGHE LOG TỪ THƯ VIỆN AUDIO
// ==========================================
void audio_info(const char *info){
    Serial.print("[AUDIO INFO] "); Serial.println(info);
}

void audio_showstation(const char *info){
    Serial.print("[AUDIO STATION] "); Serial.println(info);
}

void audio_showstreamtitle(const char *info){
    Serial.print("[AUDIO TITLE] "); Serial.println(info);
}

void audio_eof_mp3(const char *info){  // Gọi khi hát hết bài
    Serial.print("[AUDIO EOF] Het bai: "); Serial.println(info);
    s_track_ended = true; // Bật cờ để MusicTask tự chuyển bài tiếp
}