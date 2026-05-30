#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <TFT_eSPI.h> 
#include "../../include/events.h"
#include "../../include/config.h"

extern QueueHandle_t mediaQueue;
extern TFT_eSPI tft; 

// Nhận biến từ Hệ điều hành (Kernel)
extern volatile int system_state;
extern int max_score;
extern float base_paddle_speed;

// Các biến chỉ dùng nội bộ trong Game này
static const int SCR_W = 160;
static const int SCR_H = 128;
#define BG_COLOR tft.color565(0, 0, 64) 

static float ball_x, ball_y, ball_dx, ball_dy;
static float paddle_speed, ai_speed;
static int player_y, ai_y, score_ai, score_player;
static int old_ball_x, old_ball_y, old_player_y, old_ai_y;
static const int paddle_w = 4, paddle_h = 24, margin = 10;
static const float SPEED_MULTIPLIER = 1.10; 

// --- Hàm tiện ích nội bộ của Game ---
static void drawNet() {
    for (int i = 0; i < SCR_H; i += 10) tft.drawFastVLine(SCR_W / 2, i, 5, TFT_YELLOW);
}

static void drawScores() {
    tft.setTextColor(TFT_WHITE, BG_COLOR); tft.setTextSize(2);
    tft.setCursor(SCR_W / 2 - 30, 10); tft.printf("%d", score_ai);
    tft.setCursor(SCR_W / 2 + 15, 10); tft.printf("%d", score_player);
}

static void initPong() {
    ball_x = SCR_W / 2; ball_y = SCR_H / 2;
    ball_dx = 2.0; ball_dy = 2.0; 
    paddle_speed = base_paddle_speed; ai_speed = base_paddle_speed;
    player_y = (SCR_H / 2) - (paddle_h / 2); ai_y = (SCR_H / 2) - (paddle_h / 2);
    score_ai = 0; score_player = 0;
    
    tft.fillScreen(BG_COLOR);
    drawNet(); drawScores();
}

// ==========================================
// ĐÂY LÀ TIẾN TRÌNH (PROCESS) CỦA GAME
// ==========================================
void PingPongTask(void *pvParameters) {
    Serial.println("[App] Game Ping Pong dang chay tren Core 1...");
    initPong();

    while (1) {
        // 1. KIỂM TRA LỆNH KHAI TỬ TỪ HỆ ĐIỀU HÀNH
        // Nếu system_state bị đổi khác 1 (Do người dùng giữ nút HOME), Game phải lập tức thoát!
        if (system_state != 1) {
            Serial.println("[App] Ping Pong nhan lenh thoat! Dang giai phong RAM...");
            vTaskDelete(NULL); // Lệnh này sẽ tiêu diệt Task này vĩnh viễn
        }

        // 2. VẬT LÝ VÀ LOGIC GAME (Giống hệt code cũ)
        old_ball_x = (int)ball_x; old_ball_y = (int)ball_y;
        old_player_y = player_y; old_ai_y = ai_y;

        int joyY = analogRead(JOY_Y_PIN);
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
            }
        }
        if (ball_x + 4 >= SCR_W - margin - paddle_w && ball_x <= SCR_W - margin) {
            if (ball_y + 4 >= player_y && ball_y <= player_y + paddle_h) {
                ball_x = SCR_W - margin - paddle_w - 4; ball_dx = -ball_dx;
                ball_dx *= SPEED_MULTIPLIER; ball_dy *= SPEED_MULTIPLIER;
                paddle_speed *= SPEED_MULTIPLIER; ai_speed *= SPEED_MULTIPLIER;     
            }
        }

        if (ball_dx > 6.0) ball_dx = 6.0; if (ball_dx < -6.0) ball_dx = -6.0;
        if (ball_dy > 6.0) ball_dy = 6.0; if (ball_dy < -6.0) ball_dy = -6.0;

        bool isGoal = false;
        if (ball_x < 0) { score_player++; isGoal = true; } 
        else if (ball_x > SCR_W) { score_ai++; isGoal = true; } 

        if (isGoal) {
            if (score_player >= max_score || score_ai >= max_score) {
                tft.fillScreen(BG_COLOR); tft.setTextColor(TFT_YELLOW); tft.setTextSize(2);
                tft.setCursor(35, 40);
                if (score_player >= max_score) tft.print("YOU WIN!"); else tft.print("AI WINS!");
                vTaskDelay(3000 / portTICK_PERIOD_MS); 
                system_state = 0; // Đánh tín hiệu về Menu
            } else {
                ball_x = SCR_W/2; ball_y = SCR_H/2;
                ball_dx = (ball_dx > 0 ? -2.0 : 2.0); ball_dy = (random(0, 2) == 0 ? 2.0 : -2.0); 
                paddle_speed = base_paddle_speed; ai_speed = base_paddle_speed;
                player_y = (SCR_H / 2) - (paddle_h / 2); ai_y = (SCR_H / 2) - (paddle_h / 2);
                tft.fillScreen(BG_COLOR); drawNet(); drawScores();
                vTaskDelay(1000 / portTICK_PERIOD_MS); 
            }
        } else {
            if ((int)ball_x != old_ball_x || (int)ball_y != old_ball_y) tft.fillRect(old_ball_x, old_ball_y, 4, 4, BG_COLOR);
            if (player_y != old_player_y) tft.fillRect(SCR_W - margin - paddle_w, old_player_y, paddle_w, paddle_h, BG_COLOR);
            if (ai_y != old_ai_y) tft.fillRect(margin, old_ai_y, paddle_w, paddle_h, BG_COLOR);

            if (old_ball_y <= 30) drawScores();
            if (old_ball_x >= SCR_W/2 - 5 && old_ball_x <= SCR_W/2 + 5) drawNet();

            tft.fillRect((int)ball_x, (int)ball_y, 4, 4, TFT_WHITE);
            tft.fillRect(SCR_W - margin - paddle_w, player_y, paddle_w, paddle_h, TFT_WHITE); 
            tft.fillRect(margin, ai_y, paddle_w, paddle_h, TFT_WHITE); 
        }

        vTaskDelay(20 / portTICK_PERIOD_MS); 
    }
}

// Hàm này được OS gọi để khởi tạo Task Game
void AppPingPong_Start() {
    // Khai sinh một Task mới, chạy hàm PingPongTask, nằm trên Core 1
    xTaskCreatePinnedToCore(PingPongTask, "PingPong", 4096, NULL, 1, NULL, 1);
}