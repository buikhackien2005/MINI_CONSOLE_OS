#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <SD.h> // [MỚI] Bắt buộc có để dùng các lệnh duyệt File (ls, cat)

extern volatile int system_state;

void CliTask(void *pvParameters) {
    String input = "";
    bool is_cli_active = false; // "Ổ khóa" của Terminal

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
                            is_cli_active = true; // Mở khóa
                            Serial.println("\n=================================");
                            Serial.println("  MINI-OS BASH SHELL (v1.0)");
                            Serial.println("  Go 'help' de xem lenh, 'exit' de thoat");
                            Serial.println("=================================");
                            Serial.print("root@minios:~# ");
                        } else {
                            // Nếu gõ linh tinh lúc chưa mở cmd, bỏ qua không làm gì cả
                        }
                    } 
                    // ==============================================
                    // TẦNG 2: BÊN TRONG SHELL (Xử lý lệnh Ubuntu)
                    // ==============================================
                    else {
                        if (input == "exit") {
                            is_cli_active = false; // Đóng khóa
                            Serial.println("\n[Shell] Da dong Terminal.");
                        }
                        else if (input == "help") {
                            Serial.println("\n--- LENH HE THONG ---");
                            Serial.println("free   : Xem dung luong RAM");
                            Serial.println("top    : Xem trang thai CPU/OS");
                            Serial.println("kill   : Ep tat App dang chay");
                            Serial.println("\n--- LENH UBUNTU (FILE SYSTEM) ---");
                            Serial.println("ls     : Liet ke cac file tren the nho");
                            Serial.println("cat <file> : Doc noi dung file (VD: cat config.txt)");
                            Serial.println("clear  : Xoa trang man hinh Terminal");
                        }
                        else if (input == "clear") {
                            // Gửi mã ANSI để xóa sạch màn hình Terminal (giống hệt Ubuntu)
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
                        // --- MÔ PHỎNG LỆNH 'ls' (List Directory) ---
                        else if (input == "ls") {
                            Serial.println();
                            File root = SD.open("/");
                            if (!root) {
                                Serial.println("Loi: Khong the doc the SD!");
                            } else {
                                File file = root.openNextFile();
                                while (file) {
                                    if (file.isDirectory()) {
                                        Serial.print("DIR  \t ");
                                        Serial.println(file.name());
                                    } else {
                                        Serial.print("FILE \t ");
                                        Serial.print(file.name());
                                        Serial.print(" \t (");
                                        Serial.print(file.size());
                                        Serial.println(" bytes)");
                                    }
                                    file = root.openNextFile();
                                }
                            }
                        }
                        // --- MÔ PHỎNG LỆNH 'cat' (Đọc nội dung file) ---
                        else if (input.startsWith("cat ")) {
                            String fileName = "/" + input.substring(4); // Cắt lấy tên file sau chữ 'cat '
                            fileName.trim();
                            
                            File file = SD.open(fileName);
                            if (!file || file.isDirectory()) {
                                Serial.println("\ncat: " + fileName + ": Khong tim thay file");
                            } else {
                                Serial.println("\n--- " + fileName + " ---");
                                while (file.available()) {
                                    Serial.write(file.read());
                                }
                                Serial.println("\n-------------------");
                                file.close();
                            }
                        }
                        else {
                            Serial.println("\nbash: " + input + ": command not found");
                        }
                        
                        // In lại dấu nhắc lệnh của Ubuntu nếu chưa exit
                        if (is_cli_active) Serial.print("\nroot@minios:~# "); 
                    }
                    
                    input = ""; // Xóa chuỗi cũ
                }
            } else {
                input += c; 
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
}