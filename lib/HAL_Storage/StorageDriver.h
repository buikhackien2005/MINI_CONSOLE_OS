#ifndef HAL_STORAGE_H
#define HAL_STORAGE_H

#include <stdbool.h>

bool Storage_Init();
void Storage_WriteLog(const char* message);

#endif