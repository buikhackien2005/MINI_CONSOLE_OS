#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <vector>
#include "../../include/config.h"
#include "../../lib/HAL_Display/DisplayDriver.h"
#include "../../lib/HAL_Input/InputDriver.h"
#include "../../lib/HAL_Audio/AudioDriver.h"
#include <SD.h>

extern volatile int system_state;
extern volatile bool menu_selected;
extern volatile bool request_full_redraw;

std::vector<String> playlist;
static int  current_track    = 0;
static int  current_volume   = 15;    // Âm lượng hiện tại (0-20)
static bool force_redraw     = true;
static bool vol_bar_visible  = false; // Thanh volume chỉ hiện khi đang chỉnh
static unsigned long vol_bar_timer = 0; // Thời điểm bắt đầu hiện thanh
const  unsigned long VOL_BAR_SHOW_MS = 2000; // Ẩn thanh sau 2 giây

// Hàm quét thẻ nhớ tìm file MP3
static void scanMusicFiles() {
    playlist.clear();
    File dir = SD.open("/music");
    if (!dir) return;
    
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) break;
        if (!entry.isDirectory()) {
            String fullPath = entry.name(); // Trả về full path: "/music/MCKChePhu.mp3"
            String name = fullPath.substring(fullPath.lastIndexOf('/') + 1); // Chỉ lấy "MCKChePhu.mp3"
            if (name.endsWith(".mp3") || name.endsWith(".MP3")) {
                playlist.push_back(name);
            }
        }
        entry.close();
    }
    dir.close();
}

static void drawMusicUI() {
    Display_FillScreen(COLOR_BG);
    Display_DrawText("MP3 PLAYER", 25, 10, 2, COLOR_BLUE);
    Display_DrawHLine(0, 30, 160, COLOR_BLUE);

    if (playlist.empty()) {
        Display_DrawText("No MP3 files", 20, 60, 1, COLOR_RED);
        Display_DrawText("in /music folder", 10, 80, 1, COLOR_YELLOW);
        return;
    }

    // ============================================
    // THANH VOLUME DỌC — chỉ vẽ khi đang chỉnh
    // barX=119, barW=8 → cạnh phải: 119+8=127 (sát viền)
    // 20 mức × 3px = 60px chiều cao
    // ============================================
    if (vol_bar_visible) {
        const int barX    = 119;
        const int barW    = 8;
        const int barTopY = 33;
        const int barH    = 60;  // 20 × 3px
        const int unitH   = 3;

        // 1. Nền tối (phần rỗng)
        Display_DrawRect(barX, barTopY, barW, barH, 0x2104);

        // 2. Phần đầy từ dưới lên
        if (current_volume > 0) {
            int fillH = current_volume * unitH;
            Display_DrawRect(barX, barTopY + barH - fillH, barW, fillH, COLOR_GREEN);
        }

        // 3. Số volume bên dưới thanh
        int numX = (current_volume >= 10) ? barX - 2 : barX + 1;
        Display_DrawInt(current_volume, numX, barTopY + barH + 3, 1, COLOR_WHITE, COLOR_BG);
    }

    // ============================================
    // THÔNG TIN BÀI HÁT (cột trái, tránh thanh)
    // ============================================
    Display_DrawText("Playing:", 5, 38, 1, COLOR_YELLOW);
    Display_DrawText(playlist[current_track].c_str(), 5, 50, 1, COLOR_WHITE);

    // Trạng thái (Đang phát / Tạm dừng)
    if (Audio_IsPlaying()) {
        Display_DrawText(">> PLAYING", 5, 68, 1, COLOR_GREEN);
    } else {
        Display_DrawText("|| PAUSED",  5, 68, 1, COLOR_RED);
    }

    // Hướng dẫn điều khiển (bỏ ^ v:Volume vì thanh đã trực quan)
    Display_DrawHLine(0, 108, 160, 0x4208);
    Display_DrawText("< >:Next", 5,  115, 1, 0x8410);
    Display_DrawText("SW:Pause", 70, 115, 1, 0x8410);
}

void MusicTask(void *pvParameters) {
    Audio_InitI2S();
    Audio_SetVolume(current_volume); // Áp dụng âm lượng đang lưu
    vTaskDelay(100 / portTICK_PERIOD_MS); // Cho phần cứng I2S ổn định trước khi phát
    scanMusicFiles();
    force_redraw = true;
    menu_selected = false;
    
    if (!playlist.empty()) {
        String path = "/music/" + playlist[current_track];
        Audio_PlayMusic(path.c_str());
    }

    while (1) {
        // Lệnh thoát App
        if (system_state != 2) { 
            Audio_Stop();
            vTaskDelete(NULL); 
        }

        // Tự chữa lành UI khi rút thẻ
        if (request_full_redraw && system_state == 2) {
            force_redraw = true;
            request_full_redraw = false;
        }

        if (force_redraw) {
            drawMusicUI();
            force_redraw = false;
        }

        // 1. DUY TRÌ BỘ ĐỆM ÂM THANH (Rất quan trọng, phải chạy liên tục)
        Audio_Loop();

        // 2. TỰ ĐỘNG CHUYỂN BÀI KHI HẾT NHẠC
        if (Audio_IsTrackEnded() && !playlist.empty()) {
            Audio_ClearTrackEnded();
            current_track++;
            if (current_track >= (int)playlist.size()) current_track = 0;
            String path = "/music/" + playlist[current_track];
            Audio_PlayMusic(path.c_str());
            force_redraw = true;
        }

        // 3. Joystick Trái/Phải — Chuyển bài
        int joyX = Input_GetJoyX();
        if (joyX < 1000 && !playlist.empty()) { // Lùi bài
            current_track--;
            if (current_track < 0) current_track = playlist.size() - 1;
            String path = "/music/" + playlist[current_track];
            Audio_PlayMusic(path.c_str());
            force_redraw = true;
            vTaskDelay(300 / portTICK_PERIOD_MS); 
        } else if (joyX > 3000 && !playlist.empty()) { // Tiến bài
            current_track++;
            if (current_track >= playlist.size()) current_track = 0;
            String path = "/music/" + playlist[current_track];
            Audio_PlayMusic(path.c_str());
            force_redraw = true;
            vTaskDelay(300 / portTICK_PERIOD_MS);
        }

        // 4. Joystick Lên/Xuống — Âm lượng (hiện thanh khi chỉnh)
        int joyY = Input_GetJoyY();
        if (joyY < 1000) { // Lên → tăng âm lượng
            current_volume++;
            if (current_volume > 20) current_volume = 20;
            Audio_SetVolume(current_volume);
            vol_bar_visible = true;
            vol_bar_timer   = millis();
            force_redraw    = true;
            vTaskDelay(150 / portTICK_PERIOD_MS);
        } else if (joyY > 3000) { // Xuống → giảm âm lượng
            current_volume--;
            if (current_volume < 0) current_volume = 0;
            Audio_SetVolume(current_volume);
            vol_bar_visible = true;
            vol_bar_timer   = millis();
            force_redraw    = true;
            vTaskDelay(150 / portTICK_PERIOD_MS);
        }

        // 5. Quản lý Nút SW (Play/Pause)
        if (menu_selected) {
            menu_selected = false;
            Audio_PauseResume();
            force_redraw = true;
        }

        // Tự ẩn thanh volume sau VOL_BAR_SHOW_MS nếu không chỉnh thêm
        if (vol_bar_visible && millis() - vol_bar_timer > VOL_BAR_SHOW_MS) {
            vol_bar_visible = false;
            force_redraw    = true; // Redraw màn hình không có thanh
        }

        // Delay siêu nhỏ (2ms) để không chặn việc nạp dữ liệu MP3
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

void AppMusic_Start() {
    // Stack 16384: ESP32-audioI2S cần đủ bộ nhớ để decode MP3
    // Core 0: Tách riêng khỏi InputTask & WindowManagerTask (đang ở Core 1) để không tranh chấp CPU
    xTaskCreatePinnedToCore(MusicTask, "Music", 16384, NULL, 2, NULL, 0);
}