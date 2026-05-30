Mini_Console_OS/
│
├── include/                     # (DÙNG CHUNG) Các định nghĩa toàn cục
│   ├── config.h                 # Định nghĩa chân GPIO, hằng số hệ thống
│   └── events.h                 # Định nghĩa các Event/Message cho IPC (Queue)
│
├── lib/                         # [TẦNG 2] HARDWARE ABSTRACTION LAYER (HAL)
│   ├── HAL_Display/             # Bọc thư viện TFT. App gọi hàm vẽ qua đây.
│   │   ├── DisplayDriver.cpp
│   │   └── DisplayDriver.h
│   ├── HAL_Input/               # Bọc thư viện nút bấm/Joystick.
│   │   ├── InputDriver.cpp
│   │   └── InputDriver.h
│   ├── HAL_Audio/               # Bọc giao tiếp I2S/Amply.
│   └── HAL_Storage/             # Bọc giao tiếp thẻ SD / SPIFFS.
│
├── src/
│   ├── main.cpp                 # [TẦNG 1] BOOTLOADER & ENTRY POINT (Khởi tạo & gọi Kernel)
│   │
│   ├── os/                      # [TẦNG 3] KERNEL & MIDDLEWARE (Lõi hệ điều hành)
│   │   ├── system_task.cpp      # Quản lý năng lượng (Sleep Mode), cấp phát tài nguyên
│   │   ├── input_task.cpp       # Nhận tín hiệu từ HAL_Input, phân phối Event vào Queue
│   │   ├── cli_task.cpp         # Terminal/Shell giám sát RAM/CPU
│   │   └── window_manager.cpp   # (Đổi tên từ display_task) Quản lý màn hình chính (Menu) & điều hướng
│   │
│   └── apps/                    # [TẦNG 4] USER SPACE (Không gian ứng dụng)
│       ├── app_pingpong.cpp     # Logic Game Ping Pong bị cách ly hoàn toàn ở đây!
│       ├── app_pingpong.h
│       ├── app_music.cpp        # Giao diện và logic trình phát nhạc
│       └── app_music.h
│
└── platformio.ini