#ifndef HAL_AUDIO_H
#define HAL_AUDIO_H

void Audio_Init();
void Audio_PlayBeep();

// [MỚI] Các API cho App Music Player
void Audio_InitI2S();
void Audio_PlayMusic(const char* path);
void Audio_PauseResume();
void Audio_Stop();
void Audio_Loop(); // Hàm duy trì bộ đệm âm thanh
bool Audio_IsPlaying();
void Audio_SetVolume(int vol);    // Điều chỉnh âm lượng (0-20)
bool Audio_IsTrackEnded();        // Kiểm tra bài vừa hết (auto-next)
void Audio_ClearTrackEnded();     // Xóa cờ sau khi đã xử lý

#endif