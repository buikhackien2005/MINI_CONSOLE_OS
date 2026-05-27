#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <TFT_eSPI.h> 
#include "../../include/events.h"
#include "../../include/config.h"
#include "../../lib/DisplayManager/DisplayManager.h"

extern QueueHandle_t renderQueue;
extern QueueHandle_t mediaQueue;
extern TFT_eSPI tft; 

// --- THÔNG SỐ KHUNG HÌNH (XOAY NGANG) ---
const int SCR_W = 160;
const int SCR_H = 128;
#define BG_COLOR tft.color565(0, 0, 64) // Màu xanh Navy tối (giống ảnh mẫu)

// --- THÔNG SỐ GAME ---
float ball_x = SCR_W / 2, ball_y = SCR_H / 2;
float ball_dx = 2.0, ball_dy = 2.0; 
const float SPEED_MULTIPLIER = 1.15; // Tăng 15% tốc độ sau mỗi lần đập trúng vợt

int player_y = 50;  // Tọa độ vợt phải (Người chơi thật)
int ai_y = 50;      // Tọa độ vợt trái (AI Máy)
const int paddle_w = 4;
const int paddle_h = 24;
const int margin = 10; // Khoảng cách từ vợt đến mép màn hình

int score_ai = 0;
int score_player = 0;
const int MAX_SCORE = 5; // Chạm 5 điểm là kết thúc game

int old_ball_x, old_ball_y, old_player_y, old_ai_y;

// Hàm vẽ lưới phân cách ở giữa
void drawNet() {
    for (int i = 0; i < SCR_H; i += 10) {
        tft.drawFastVLine(SCR_W / 2, i, 5, TFT_YELLOW);
    }
}

// Hàm vẽ điểm số
void drawScores() {
    tft.setTextColor(TFT_WHITE, BG_COLOR);
    tft.setTextSize(2);
    tft.setCursor(SCR_W / 2 - 30, 10);
    tft.printf("%d", score_ai);
    tft.setCursor(SCR_W / 2 + 15, 10);
    tft.printf("%d", score_player);
}

// Hàm reset bóng khi có người ghi bàn
void resetBall() {
    ball_x = SCR_W / 2;
    ball_y = SCR_H / 2;
    ball_dx = (ball_dx > 0 ? -2.0 : 2.0); // Bắn bóng về phía người vừa ghi điểm
    ball_dy = (random(0, 2) == 0 ? 2.0 : -2.0); // Hướng nảy ngẫu nhiên lên/xuống
    
    tft.fillScreen(BG_COLOR);
    drawNet();
    drawScores();
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Nghỉ 1 giây trước khi phát bóng
}

void DisplayTask(void *pvParameters) {
    Serial.println("[Task] Classic Ping Pong (Core 1) khởi động...");
    DisplayManager::init();

    // 1. ÉP XOAY NGANG MÀN HÌNH (Landscape)
    tft.setRotation(1); 
    
    tft.fillScreen(BG_COLOR);
    drawNet();
    drawScores();

    while (1) {
        // 2. XÓA VẾT CŨ BẰNG MÀU NỀN
        tft.fillRect(old_ball_x, old_ball_y, 4, 4, BG_COLOR);
        tft.fillRect(SCR_W - margin - paddle_w, old_player_y, paddle_w, paddle_h, BG_COLOR);
        tft.fillRect(margin, old_ai_y, paddle_w, paddle_h, BG_COLOR);

        // Vẽ dặm lại nét đứt ở giữa nếu quả bóng vừa bay ngang qua làm đứt nét
        if (old_ball_x >= SCR_W/2 - 4 && old_ball_x <= SCR_W/2 + 4) drawNet();

        // 3. ĐỌC NÚT BẤM CỦA NGƯỜI CHƠI (Bên Phải - Dùng UP/DOWN)
        if (digitalRead(BTN_UP_PIN) == LOW) player_y -= 4;
        if (digitalRead(BTN_DOWN_PIN) == LOW) player_y += 4;
        
        // Chặn không cho vợt lọt ra ngoài màn hình
        if (player_y < 0) player_y = 0;
        if (player_y > SCR_H - paddle_h) player_y = SCR_H - paddle_h;

        // 4. LOGIC AI TỰ ĐỘNG CHƠI (Bên Trái)
        // AI sẽ liên tục trượt theo tọa độ y của quả bóng, tốc độ trễ hơn bóng một chút để có thể bị thua
        if (ai_y + paddle_h / 2 < ball_y) ai_y += 2;
        else ai_y -= 2;
        
        if (ai_y < 0) ai_y = 0;
        if (ai_y > SCR_H - paddle_h) ai_y = SCR_H - paddle_h;

        // 5. TÍNH TOÁN VẬT LÝ BÓNG
        ball_x += ball_dx;
        ball_y += ball_dy;

        // Bóng đập trần nhà hoặc sàn nhà
        if (ball_y <= 0 || ball_y >= SCR_H - 4) ball_dy = -ball_dy;

        // Bóng đập Vợt AI (Bên Trái)
        if (ball_x <= margin + paddle_w && ball_x >= margin) {
            if (ball_y + 4 >= ai_y && ball_y <= ai_y + paddle_h) {
                ball_x = margin + paddle_w; // Đẩy quả bóng ra khỏi vợt tránh kẹt
                ball_dx = -ball_dx;
                ball_dx *= SPEED_MULTIPLIER; // Ép xung tốc độ bóng bay
                ball_dy *= SPEED_MULTIPLIER;
                MediaEvent beepCmd; beepCmd.cmdType = 1; xQueueSend(mediaQueue, &beepCmd, 0); // Kêu Bíp
            }
        }

        // Bóng đập Vợt Người Chơi (Bên Phải)
        if (ball_x + 4 >= SCR_W - margin - paddle_w && ball_x <= SCR_W - margin) {
            if (ball_y + 4 >= player_y && ball_y <= player_y + paddle_h) {
                ball_x = SCR_W - margin - paddle_w - 4;
                ball_dx = -ball_dx;
                ball_dx *= SPEED_MULTIPLIER;
                ball_dy *= SPEED_MULTIPLIER;
                MediaEvent beepCmd; beepCmd.cmdType = 1; xQueueSend(mediaQueue, &beepCmd, 0); // Kêu Bíp
            }
        }

        // Khóa tốc độ tối đa để bóng không tàng hình xuyên qua vợt do bay quá nhanh
        if (ball_dx > 6.0) ball_dx = 6.0;
        if (ball_dx < -6.0) ball_dx = -6.0;
        if (ball_dy > 6.0) ball_dy = 6.0;
        if (ball_dy < -6.0) ball_dy = -6.0;

        // 6. LUẬT GHI BÀN & GAME OVER
        bool isGoal = false;
        if (ball_x < 0) { score_player++; isGoal = true; } // Bóng lọt gôn trái -> Người ăn điểm
        else if (ball_x > SCR_W) { score_ai++; isGoal = true; } // Bóng lọt gôn phải -> AI ăn điểm

        if (isGoal) {
            if (score_player >= MAX_SCORE || score_ai >= MAX_SCORE) {
                // MÀN HÌNH GAME OVER XOAY NGANG
                tft.fillScreen(BG_COLOR);
                tft.setTextColor(TFT_YELLOW); tft.setTextSize(2);
                tft.setCursor(35, 40);
                if (score_player >= MAX_SCORE) tft.print("YOU WIN!");
                else tft.print("AI WINS!");

                tft.setTextColor(TFT_WHITE); tft.setTextSize(1);
                tft.setCursor(35, 75);
                tft.printf("Final Score: %d - %d", score_ai, score_player);

                // Gửi đa nhiệm lưu thẻ nhớ (Core 0 chạy ngầm)
                MediaEvent logCmd; logCmd.cmdType = 2;
                sprintf(logCmd.logData, "Pong Game Over! Score: AI %d - Player %d", score_ai, score_player);
                xQueueSend(mediaQueue, &logCmd, 0);

                vTaskDelay(4000 / portTICK_PERIOD_MS); // Chờ 4 giây trước khi ván mới
                score_player = 0;
                score_ai = 0;
            }
            resetBall();
        } else {
            // LƯU LẠI TỌA ĐỘ ĐỂ XÓA VÀO VÒNG LẶP SAU
            old_ball_x = (int)ball_x;
            old_ball_y = (int)ball_y;
            old_player_y = player_y;
            old_ai_y = ai_y;

            // VẼ TRẠNG THÁI MỚI (Bóng và 2 vợt trắng)
            tft.fillRect(old_ball_x, old_ball_y, 4, 4, TFT_WHITE);
            tft.fillRect(SCR_W - margin - paddle_w, player_y, paddle_w, paddle_h, TFT_WHITE); 
            tft.fillRect(margin, ai_y, paddle_w, paddle_h, TFT_WHITE); 
        }

        // 7. ĐIỀU TỐC GAME ENGINE (~50 FPS)
        vTaskDelay(20 / portTICK_PERIOD_MS); 
    }
}