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
const float SPEED_MULTIPLIER = 1.10; // Tăng 10% tốc độ sau mỗi lần đập trúng vợt

// [MỚI] Thêm biến tốc độ động cho 2 vợt
float paddle_speed = 2.0; // Tốc độ cơ bản của người chơi
float ai_speed = 2.0;     // Tốc độ cơ bản của AI

int player_y = 50;  // Tọa độ vợt phải (Người chơi thật)
int ai_y = 50;      // Tọa độ vợt trái (AI Máy)
const int paddle_w = 4;
const int paddle_h = 24;
const int margin = 10; // Khoảng cách từ vợt đến mép màn hình

int score_ai = 0;
int score_player = 0;
const int MAX_SCORE = 7; // Chạm 7 điểm là kết thúc game

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

void resetBall() {
    ball_x = SCR_W / 2;
    ball_y = SCR_H / 2;
    ball_dx = (ball_dx > 0 ? -2.0 : 2.0); 
    ball_dy = (random(0, 2) == 0 ? 2.0 : -2.0); 
    
    // Reset tốc độ vợt về mặc định
    paddle_speed = 2.0;
    ai_speed = 2.0;
    
    // [CẬP NHẬT] Đưa 2 vợt về vị trí chính giữa màn hình
    // Lấy chiều cao màn hình chia đôi, trừ đi một nửa chiều cao của vợt để căn giữa tuyệt đối
    player_y = (SCR_H / 2) - (paddle_h / 2);
    ai_y = (SCR_H / 2) - (paddle_h / 2);
    
    // Vẽ lại màn hình mới
    tft.fillScreen(BG_COLOR);
    drawNet();
    drawScores();
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
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

        // [MỚI] VẼ LẠI LƯỚI VÀ ĐIỂM SỐ NGAY LẬP TỨC 
        // Để đảm bảo chúng luôn luôn nằm trên cùng (Top layer), không bị bóng bôi đen mất
        drawNet();
        drawScores();
        
        // Vẽ dặm lại nét đứt ở giữa nếu quả bóng vừa bay ngang qua làm đứt nét
        if (old_ball_x >= SCR_W/2 - 4 && old_ball_x <= SCR_W/2 + 4) drawNet();

        // 3. ĐỌC JOYSTICK ANALOG
        int joyY = analogRead(JOY_Y_PIN);
        if (joyY < 1500) player_y -= (int)paddle_speed; // Ép kiểu float về int
        else if (joyY > 2500) player_y += (int)paddle_speed;
        
        if (player_y < 0) player_y = 0;
        if (player_y > SCR_H - paddle_h) player_y = SCR_H - paddle_h;

        // 4. LOGIC AI TỰ ĐỘNG CHƠI
        if (ai_y + paddle_h / 2 < ball_y) ai_y += (int)ai_speed;
        else ai_y -= (int)ai_speed;
        
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
                ball_x = margin + paddle_w; 
                ball_dx = -ball_dx;
                
                // Tăng tốc toàn bộ hệ thống
                ball_dx *= SPEED_MULTIPLIER; 
                ball_dy *= SPEED_MULTIPLIER;
                paddle_speed *= SPEED_MULTIPLIER; // [MỚI] Tăng tốc vợt bạn
                ai_speed *= SPEED_MULTIPLIER;     // [MỚI] Tăng tốc vợt AI
                
                MediaEvent beepCmd; beepCmd.cmdType = 1; xQueueSend(mediaQueue, &beepCmd, 0); 
            }
        }

        // Bóng đập Vợt Người Chơi (Bên Phải)
        if (ball_x + 4 >= SCR_W - margin - paddle_w && ball_x <= SCR_W - margin) {
            if (ball_y + 4 >= player_y && ball_y <= player_y + paddle_h) {
                ball_x = SCR_W - margin - paddle_w - 4;
                ball_dx = -ball_dx;
                
                // Tăng tốc toàn bộ hệ thống
                ball_dx *= SPEED_MULTIPLIER;
                ball_dy *= SPEED_MULTIPLIER;
                paddle_speed *= SPEED_MULTIPLIER; // [MỚI] Tăng tốc vợt bạn
                ai_speed *= SPEED_MULTIPLIER;     // [MỚI] Tăng tốc vợt AI
                
                MediaEvent beepCmd; beepCmd.cmdType = 1; xQueueSend(mediaQueue, &beepCmd, 0); 
            }
        }

        // Khóa tốc độ tối đa (Rất quan trọng!)
        if (ball_dx > 6.0) ball_dx = 6.0;
        if (ball_dx < -6.0) ball_dx = -6.0;
        if (ball_dy > 6.0) ball_dy = 6.0;
        if (ball_dy < -6.0) ball_dy = -6.0;
        
        // [MỚI] Bóng bị khóa ở 6.0 (Gấp 3 lần mức ban đầu 2.0). 
        // Do đó Vợt cũng chỉ được tăng tối đa gấp 3 lần để tránh bay ra ngoài vũ trụ.
        if (paddle_speed > 12.0) paddle_speed = 12.0; // 4.0 x 3 = 12.0
        if (ai_speed > 6.0) ai_speed = 6.0;           // 2.0 x 3 = 6.0

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