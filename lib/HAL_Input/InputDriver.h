#ifndef HAL_INPUT_H
#define HAL_INPUT_H

// Các hàm API tiêu chuẩn để các App và OS gọi
void Input_Init();
int Input_GetJoyY();
bool Input_IsHomePressed();

#endif