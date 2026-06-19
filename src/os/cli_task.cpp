#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <SD.h> 
#include <LittleFS.h> // [MỚI] Bổ sung thư viện quản lý Ổ C:

extern volatile int system_state;

void CliTask(void *pvParameters) {
    String input = "";
    bool is_cli_active = false; 

    Serial.println("[OS] Serial Port san sang. Go 'cmd' de mo Terminal.");

    while (1) {
        if (Serial.available() > 0) {
            char c = Serial.read();
            
            if (c == '\n' || c == '\r') {
                if (input.length() > 0) {
                    input.trim();
                    
                    // ==============================================
                    // TẦNG 1: NGOÀI CỬA (Chỉ chờ lệnh 'cmd')
                    // ==============================================
                    if (!is_cli_active) {
                        if (input == "cmd") {
                            is_cli_active = true;
                            Serial.println("\n=================================");
                            Serial.println("  MINI-OS BASH SHELL (v2.0 - Dual Drive)");
                            Serial.println("  Go 'help' de xem lenh, 'exit' de thoat");
                            Serial.println("=================================");
                            Serial.print("root@minios:~# ");
                        }
                    } 
                    // ==============================================
                    // TẦNG 2: BÊN TRONG SHELL (Xử lý lệnh Ubuntu)
                    // ==============================================
                    else {
                        if (input == "exit") {
                            is_cli_active = false; 
                            Serial.println("\n[Shell] Da dong Terminal.");
                        }
                        else if (input == "help") {
                            Serial.println("\n--- LENH HE THONG ---");
                            Serial.println("free   : Xem dung luong RAM");
                            Serial.println("top    : Xem trang thai CPU/OS");
                            Serial.println("kill   : Ep tat App dang chay");
                            Serial.println("\n--- LENH FILE SYSTEM ---");
                            Serial.println("ls     : Liet ke file tren RootFS va SD Card");
                            Serial.println("cat <file> : Doc noi dung file (VD: cat /syslog.txt)");
                            Serial.println("clear  : Xoa trang man hinh Terminal");
                        }
                        else if (input == "clear") {
                            Serial.print("\033[2J\033[H"); 
                        }
                        else if (input == "free") {
                            Serial.printf("\nRAM con trong: %d bytes\n", ESP.getFreeHeap());
                        }
                        else if (input == "top") {
                            Serial.printf("\nSystem State: %d\n", system_state);
                        }
                        else if (input == "kill") {
                            Serial.println("\nDang ep dong App...");
                            system_state = 0;
                        }
                        // ==========================================
                        // [MỚI] LỆNH 'ls' THÔNG MINH (Quét 2 ổ đĩa)
                        // ==========================================
                        else if (input == "ls") {
                            Serial.println("\n--- [ ROOT FS (Bo nho trong) ] ---");
                            File root1 = LittleFS.open("/");
                            if (root1) {
                                File file = root1.openNextFile();
                                if (!file) Serial.println("(Thu muc trong)");
                                while (file) {
                                    Serial.printf("FILE \t %s \t (%d bytes)\n", file.name(), file.size());
                                    file = root1.openNextFile();
                                }
                            }

                            Serial.println("\n--- [ SD CARD (O cam ngoai) ] ---");
                            File root2 = SD.open("/");
                            if (root2) {
                                File file = root2.openNextFile();
                                if (!file) Serial.println("(Thu muc trong)");
                                while (file) {
                                    Serial.printf("FILE \t %s \t (%d bytes)\n", file.name(), file.size());
                                    file = root2.openNextFile();
                                }
                            } else {
                                Serial.println("(Khong co the SD)");
                            }
                        }
                        // ==========================================
                        // [MỚI] LỆNH 'cat' THÔNG MINH (Tìm trên 2 ổ)
                        // ==========================================
                        else if (input.startsWith("cat ")) {
                            String fileName = input.substring(4); 
                            fileName.trim();
                            // Tự động thêm dấu '/' ở đầu nếu người dùng quên gõ
                            if (!fileName.startsWith("/")) fileName = "/" + fileName; 
                            
                            // Ưu tiên tìm trong Root FS trước
                            File file = LittleFS.open(fileName, FILE_READ);
                            
                            // Nếu không có trong RootFS, tìm tiếp ở thẻ SD
                            if (!file || file.isDirectory()) {
                                file = SD.open(fileName, FILE_READ);
                            }

                            if (!file || file.isDirectory()) {
                                Serial.println("\ncat: " + fileName + ": Khong tim thay file tren bat ky o dia nao!");
                            } else {
                                Serial.println("\n--- Doc tu: " + fileName + " ---");
                                while (file.available()) {
                                    Serial.write(file.read());
                                }
                                Serial.println("\n-----------------------------");
                                file.close();
                            }
                        }
                        else {
                            Serial.println("\nbash: " + input + ": command not found");
                        }
                        
                        if (is_cli_active) Serial.print("\nroot@minios:~# "); 
                    }
                    input = ""; 
                }
            } else {
                input += c; 
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
}