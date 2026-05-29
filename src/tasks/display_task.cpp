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

extern volatile int system_state;
extern volatile bool menu_selected;
extern int max_score;
extern float base_paddle_speed;
extern volatile unsigned long lastActivityTime;

// Kích thước & Màu sắc
const int SCR_W = 160;
const int SCR_H = 128;
#define BG_COLOR tft.color565(0, 0, 64) 

// Biến cho Game Pong
float ball_x, ball_y, ball_dx, ball_dy;
float paddle_speed, ai_speed;
int player_y, ai_y;
int score_ai, score_player;
int old_ball_x, old_ball_y, old_player_y, old_ai_y;
const int paddle_w = 4, paddle_h = 24, margin = 10;
const float SPEED_MULTIPLIER = 1.10; 

// Biến cho Menu
int menu_index = 0; 
bool force_redraw_menu = true;
bool force_redraw_pong = false;

// --- HÀM VẼ GIAO DIỆN MENU ---
void drawMenu() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN); tft.setTextSize(2);
    tft.setCursor(20, 10);
    tft.print("MINI OS");
    tft.drawFastHLine(0, 30, SCR_W, TFT_GREEN);

    tft.setTextSize(1);
    // Mục 1: Ping Pong
    if (menu_index == 0) {
        tft.fillRect(10, 45, 140, 20, TFT_BLUE);
        tft.setTextColor(TFT_WHITE);
    } else {
        tft.setTextColor(TFT_LIGHTGREY);
    }
    tft.setCursor(15, 50); tft.print("> 1. Game Ping Pong");

    // Mục 2: Máy phát nhạc
    if (menu_index == 1) {
        tft.fillRect(10, 75, 140, 20, TFT_BLUE);
        tft.setTextColor(TFT_WHITE);
    } else {
        tft.setTextColor(TFT_LIGHTGREY);
    }
    tft.setCursor(15, 80); tft.print("> 2. Music Player");
    
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(10, 110); tft.print("Giu 'SW' 1.5s de Thoat App");
}

// --- HÀM KHỞI TẠO LẠI PONG TỪ ĐẦU ---
void initPong() {
    ball_x = SCR_W / 2; ball_y = SCR_H / 2;
    ball_dx = 2.0; ball_dy = 2.0; 
    paddle_speed = base_paddle_speed; ai_speed = base_paddle_speed;
    player_y = (SCR_H / 2) - (paddle_h / 2);
    ai_y = (SCR_H / 2) - (paddle_h / 2);
    score_ai = 0; score_player = 0;
    force_redraw_pong = true;
}

// ==========================================
// TRÁI TIM CỦA GIAO DIỆN (WINDOW MANAGER)
// ==========================================
void DisplayTask(void *pvParameters) {
    Serial.println("[Task] Window Manager (Core 1) khoi dong...");

    while (1) {
        // ----------------------------------------
        // TRẠNG THÁI 0: MÀN HÌNH CHÍNH (LAUNCHER)
        // ----------------------------------------
        if (system_state == 0) {
            if (force_redraw_menu) { drawMenu(); force_redraw_menu = false; }

            // Đọc Joystick để dịch chuyển con trỏ Menu
            int joyY = analogRead(JOY_Y_PIN);
            if (joyY < 1000 && menu_index > 0) {
                menu_index--; drawMenu(); lastActivityTime = millis();
                vTaskDelay(200 / portTICK_PERIOD_MS); // Chống dội cuộn trang
            } else if (joyY > 3000 && menu_index < 1) {
                menu_index++; drawMenu(); lastActivityTime = millis();
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }

            // Nếu người dùng ấn nút Chọn
            if (menu_selected) {
                menu_selected = false;
                if (menu_index == 0) {
                    Serial.println("[OS] Dang mo Game Ping Pong...");
                    initPong();
                    system_state = 1; // Nhảy sang trạng thái Pong
                } else if (menu_index == 1) {
                    Serial.println("[OS] Dang mo Music Player...");
                    tft.fillScreen(TFT_NAVY);
                    tft.setTextColor(TFT_WHITE); tft.setTextSize(2);
                    tft.setCursor(25, 40); tft.print("MP3 PLAYER");
                    tft.setTextSize(1); tft.setCursor(20, 70); tft.print("(Chua co the SD MP3)");
                    system_state = 2; // Nhảy sang trạng thái Music
                }
            }
        } 
        
        // ----------------------------------------
        // TRẠNG THÁI 1: GAME PING PONG ĐANG CHẠY
        // ----------------------------------------
        else if (system_state == 1) {
            if (force_redraw_pong) {
                tft.fillScreen(BG_COLOR);
                for (int i=0; i<SCR_H; i+=10) tft.drawFastVLine(SCR_W/2, i, 5, TFT_YELLOW);
                tft.setTextColor(TFT_WHITE, BG_COLOR); tft.setTextSize(2);
                tft.setCursor(SCR_W/2 - 30, 10); tft.printf("%d", score_ai);
                tft.setCursor(SCR_W/2 + 15, 10); tft.printf("%d", score_player);
                force_redraw_pong = false;
            }

            // --- Toàn bộ Logic Game Pong của bạn ghép vào đây ---
            old_ball_x = (int)ball_x; old_ball_y = (int)ball_y;
            old_player_y = player_y; old_ai_y = ai_y;

            int joyY = analogRead(JOY_Y_PIN);
            if (joyY < 1500) { player_y -= (int)paddle_speed; lastActivityTime = millis(); }
            else if (joyY > 2500) { player_y += (int)paddle_speed; lastActivityTime = millis(); }
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
                    
                    // Chơi xong 1 ván -> Ép quay về Menu chính!
                    system_state = 0;
                } else {
                    ball_x = SCR_W/2; ball_y = SCR_H/2;
                    ball_dx = (ball_dx > 0 ? -2.0 : 2.0); ball_dy = (random(0, 2) == 0 ? 2.0 : -2.0); 
                    paddle_speed = base_paddle_speed; ai_speed = base_paddle_speed;
                    player_y = (SCR_H / 2) - (paddle_h / 2);
                    ai_y = (SCR_H / 2) - (paddle_h / 2);
                    force_redraw_pong = true;
                    vTaskDelay(1000 / portTICK_PERIOD_MS); 
                }
            } else {
                if ((int)ball_x != old_ball_x || (int)ball_y != old_ball_y) tft.fillRect(old_ball_x, old_ball_y, 4, 4, BG_COLOR);
                if (player_y != old_player_y) tft.fillRect(SCR_W - margin - paddle_w, old_player_y, paddle_w, paddle_h, BG_COLOR);
                if (ai_y != old_ai_y) tft.fillRect(margin, old_ai_y, paddle_w, paddle_h, BG_COLOR);

                if (old_ball_y <= 30) {
                    tft.setTextColor(TFT_WHITE, BG_COLOR); tft.setTextSize(2);
                    tft.setCursor(SCR_W/2 - 30, 10); tft.printf("%d", score_ai);
                    tft.setCursor(SCR_W/2 + 15, 10); tft.printf("%d", score_player);
                }
                if (old_ball_x >= SCR_W/2 - 5 && old_ball_x <= SCR_W/2 + 5) {
                    for (int i=0; i<SCR_H; i+=10) tft.drawFastVLine(SCR_W/2, i, 5, TFT_YELLOW);
                }

                tft.fillRect((int)ball_x, (int)ball_y, 4, 4, TFT_WHITE);
                tft.fillRect(SCR_W - margin - paddle_w, player_y, paddle_w, paddle_h, TFT_WHITE); 
                tft.fillRect(margin, ai_y, paddle_w, paddle_h, TFT_WHITE); 
            }
        }
        
        // ----------------------------------------
        // TRẠNG THÁI 2: TRÌNH PHÁT NHẠC
        // ----------------------------------------
        else if (system_state == 2) {
            // App này hiện tại chỉ hiển thị tĩnh chờ bạn lắp module thẻ nhớ để code sau
            // Logic thoát app sẽ do InputTask đảm nhận (Bạn giữ nút SW 1.5s là thoát)
        }

        // --- QUẢN LÝ CHUYỂN ĐỔI TRẠNG THÁI (CHỐNG GIẬT MÀN HÌNH) ---
        static int last_system_state = 0;
        if (system_state != last_system_state) {
            if (system_state == 0) {
                // Chỉ yêu cầu vẽ lại Menu 1 lần duy nhất khi vừa thoát App
                force_redraw_menu = true; 
                menu_index = 0; // Đưa con trỏ về mục đầu tiên
            }
            last_system_state = system_state; // Lưu lại trạng thái để không bị lặp lại
        }

        vTaskDelay(20 / portTICK_PERIOD_MS); // Điều tốc toàn bộ OS (~50 FPS)
    }
}