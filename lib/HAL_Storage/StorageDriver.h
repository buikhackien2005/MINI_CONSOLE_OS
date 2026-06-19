#ifndef HAL_STORAGE_H
#define HAL_STORAGE_H

#include <stdbool.h>

bool Storage_Init_RootFS();  // Khởi tạo "Ổ C:" (Bộ nhớ trong của ESP32)
bool Storage_Init_SD();      // Khởi tạo "Ổ D:" (Thẻ nhớ SD cắm ngoài)
void Storage_WriteLog(const char* message);
void Storage_LoadConfig();   // [MỚI] Hàm chuyên dụng để đọc file config
void Storage_SaveConfig();   // [MỚI] Hàm chuyên dụng để lưu file config
int Storage_CheckHotSwap(); // [MỚI] Trả về 1 (Vừa cắm), -1 (Vừa rút), 0 (Không đổi)

#endif