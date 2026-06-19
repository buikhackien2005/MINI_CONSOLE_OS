#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../include/config.h"

// [MỚI] CHỈ IMPORT LỚP TRỪU TƯỢNG (HAL), TUYỆT ĐỐI KHÔNG DÙNG TFT_eSPI Ở ĐÂY
#include "../../lib/HAL_Display/DisplayDriver.h"
#include "../../lib/HAL_Input/InputDriver.h"

#include "../../include/events.h"   // Lấy cấu trúc lá đơn (MediaEvent)
extern QueueHandle_t mediaQueue;    // Lấy hòm thư của Hệ điều hành

extern volatile int system_state;
extern int max_score;
extern float base_paddle_speed;
extern volatile bool request_full_redraw;

static const int SCR_W = 160;
static const int SCR_H = 128;

static float ball_x, ball_y, ball_dx, ball_dy;
static float paddle_speed, ai_speed;
static int player_y, ai_y, score_ai, score_player;
static int old_ball_x, old_ball_y, old_player_y, old_ai_y;
static const int paddle_w = 4, paddle_h = 24, margin = 10;
static const float SPEED_MULTIPLIER = 1.10; 

static void drawNet() {
    for (int i = 0; i < SCR_H; i += 10) Display_DrawVLine(SCR_W / 2, i, 5, COLOR_YELLOW);
}

static void drawScores() {
    Display_DrawInt(score_ai, SCR_W / 2 - 30, 10, 2, COLOR_WHITE, COLOR_BG);
    Display_DrawInt(score_player, SCR_W / 2 + 15, 10, 2, COLOR_WHITE, COLOR_BG);
}

static void initPong() {
    ball_x = SCR_W / 2; ball_y = SCR_H / 2;
    ball_dx = 2.0; ball_dy = 2.0; 
    paddle_speed = base_paddle_speed; ai_speed = base_paddle_speed;
    player_y = (SCR_H / 2) - (paddle_h / 2); ai_y = (SCR_H / 2) - (paddle_h / 2);
    score_ai = 0; score_player = 0;
    
    Display_FillScreen(COLOR_BG);
    drawNet(); drawScores();
}

void PingPongTask(void *pvParameters) {
    initPong();

    while (1) {
        if (system_state != 1) { vTaskDelete(NULL); }

        // [MỚI] Tự chữa lành UI cho Ping Pong
        if (request_full_redraw && system_state == 1) {
            Display_FillScreen(COLOR_BG);
            drawNet(); 
            drawScores();
            // Ép vẽ lại vợt và bóng
            old_ball_x = -1; old_player_y = -1; old_ai_y = -1; 
            request_full_redraw = false;
        }

        old_ball_x = (int)ball_x; old_ball_y = (int)ball_y;
        old_player_y = player_y; old_ai_y = ai_y;

        // [MỚI] Sử dụng API Input thay vì analogRead
        int joyY = Input_GetJoyY();
        if (joyY < 1500) player_y -= (int)paddle_speed; 
        else if (joyY > 2500) player_y += (int)paddle_speed;
        if (player_y < 0) player_y = 0; if (player_y > SCR_H - paddle_h) player_y = SCR_H - paddle_h;

        if (ai_y + paddle_h/2 < ball_y) ai_y += (int)ai_speed;
        else if (ai_y + paddle_h/2 > ball_y) ai_y -= (int)ai_speed; 
        if (ai_y < 0) ai_y = 0; if (ai_y > SCR_H - paddle_h) ai_y = SCR_H - paddle_h;

        ball_x += ball_dx; ball_y += ball_dy;
        if (ball_y <= 0) { ball_y = 0; ball_dy = -ball_dy; } 
        else if (ball_y >= SCR_H - 4) { ball_y = SCR_H - 4; ball_dy = -ball_dy; }

        if (ball_x <= margin + paddle_w && ball_x >= margin) {
            if (ball_y + 4 >= ai_y && ball_y <= ai_y + paddle_h) {
                ball_x = margin + paddle_w; ball_dx = -ball_dx;
                ball_dx *= SPEED_MULTIPLIER; ball_dy *= SPEED_MULTIPLIER;
                paddle_speed *= SPEED_MULTIPLIER; ai_speed *= SPEED_MULTIPLIER;
                
                // [MỚI - TẦNG 3] Gửi yêu cầu phát tiếng Bíp cho OS
                MediaEvent beepCmd; 
                beepCmd.cmdType = 1; 
                xQueueSend(mediaQueue, &beepCmd, 0);
            }
        }
        if (ball_x + 4 >= SCR_W - margin - paddle_w && ball_x <= SCR_W - margin) {
            if (ball_y + 4 >= player_y && ball_y <= player_y + paddle_h) {
                ball_x = SCR_W - margin - paddle_w - 4; ball_dx = -ball_dx;
                ball_dx *= SPEED_MULTIPLIER; ball_dy *= SPEED_MULTIPLIER;
                paddle_speed *= SPEED_MULTIPLIER; ai_speed *= SPEED_MULTIPLIER;
                
                // [MỚI - TẦNG 3] Gửi yêu cầu phát tiếng Bíp cho OS
                MediaEvent beepCmd; 
                beepCmd.cmdType = 1; 
                xQueueSend(mediaQueue, &beepCmd, 0);
            }
        }

        if (ball_dx > 6.0) ball_dx = 6.0; if (ball_dx < -6.0) ball_dx = -6.0;
        if (ball_dy > 6.0) ball_dy = 6.0; if (ball_dy < -6.0) ball_dy = -6.0;

        bool isGoal = false;
        if (ball_x < 0) { score_player++; isGoal = true; } 
        else if (ball_x > SCR_W) { score_ai++; isGoal = true; } 

        if (isGoal) {
            if (score_player >= max_score || score_ai >= max_score) {
                Display_FillScreen(COLOR_BG);
                if (score_player >= max_score) Display_DrawText("YOU WIN!", 35, 40, 2, COLOR_YELLOW);
                else Display_DrawText("AI WINS!", 35, 40, 2, COLOR_YELLOW);

                // [MỚI - TẦNG 3] Gửi yêu cầu ghi lịch sử trận đấu xuống thẻ nhớ
                MediaEvent logCmd; 
                logCmd.cmdType = 2;
                sprintf(logCmd.logData, "Pong Game Over! Score: AI %d - Player %d", score_ai, score_player);
                xQueueSend(mediaQueue, &logCmd, 0);
                
                vTaskDelay(3000 / portTICK_PERIOD_MS); 
                system_state = 0; 
            } else {
                ball_x = SCR_W/2; ball_y = SCR_H/2;
                ball_dx = (ball_dx > 0 ? -2.0 : 2.0); ball_dy = (random(0, 2) == 0 ? 2.0 : -2.0); 
                paddle_speed = base_paddle_speed; ai_speed = base_paddle_speed;
                player_y = (SCR_H / 2) - (paddle_h / 2); ai_y = (SCR_H / 2) - (paddle_h / 2);
                Display_FillScreen(COLOR_BG); drawNet(); drawScores();
                vTaskDelay(1000 / portTICK_PERIOD_MS); 
            }
        } else {
            // [MỚI] Xóa vết cũ và vẽ lại vật thể mới bằng Display API
            if ((int)ball_x != old_ball_x || (int)ball_y != old_ball_y) Display_DrawRect(old_ball_x, old_ball_y, 4, 4, COLOR_BG);
            if (player_y != old_player_y) Display_DrawRect(SCR_W - margin - paddle_w, old_player_y, paddle_w, paddle_h, COLOR_BG);
            if (ai_y != old_ai_y) Display_DrawRect(margin, old_ai_y, paddle_w, paddle_h, COLOR_BG);

            if (old_ball_y <= 30) drawScores();
            if (old_ball_x >= SCR_W/2 - 5 && old_ball_x <= SCR_W/2 + 5) drawNet();

            Display_DrawRect((int)ball_x, (int)ball_y, 4, 4, COLOR_WHITE);
            Display_DrawRect(SCR_W - margin - paddle_w, player_y, paddle_w, paddle_h, COLOR_WHITE); 
            Display_DrawRect(margin, ai_y, paddle_w, paddle_h, COLOR_WHITE); 
        }

        vTaskDelay(20 / portTICK_PERIOD_MS); 
    }
}

void AppPingPong_Start() {
    xTaskCreatePinnedToCore(PingPongTask, "PingPong", 4096, NULL, 1, NULL, 1);
}