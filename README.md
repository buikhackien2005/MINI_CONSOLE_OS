# 🎮 Xây dựng kiến trúc Mini-Console OS (Ứng dụng trên ESP32)

## 📌 1. Giới thiệu đề tài

Dự án **Mini-Console OS** là một bài tập lớn thuộc môn Hệ điều hành nhúng, tập trung vào việc nghiên cứu và thiết kế kiến trúc phần mềm lõi cho một thiết bị giải trí cầm tay. Thay vì sử dụng vòng lặp siêu ngắt (Super-loop) tuần tự truyền thống, dự án này áp dụng triệt để các khái niệm của Hệ điều hành thời gian thực (**FreeRTOS**) trên nền tảng vi điều khiển lõi kép (Dual-core MCU).

**Mục tiêu cốt lõi của hệ thống:**
* **Phân bổ lõi (Core Affinity):** Chia tải các tác vụ I/O nặng (Đọc thẻ nhớ, Giải mã âm thanh) và các tác vụ UI/Logic sang hai nhân CPU độc lập.
* **Quản lý đa nhiệm (Multitasking):** Vận hành song song 4 tác vụ chính (System, Display, Input, Audio) với cơ chế phân quyền ưu tiên (Priority) nghiêm ngặt để tránh hiện tượng "đói" tài nguyên (Starvation).
* **Giao tiếp liên tác vụ (Inter-task Communication - IPC):** Giải quyết bài toán đồng bộ hóa và chống xung đột phần cứng (Bus Contention) bằng Message Queues, Semaphores và Mutex.

---

## 👥 2. Thành viên thực hiện

| STT | Họ và Tên | Mã số sinh viên (MSSV) | Phân công phụ trách |
| :---: | :--- | :--- | :--- |
| **1** | [Điền Họ và Tên của bạn] | [Điền MSSV] | Trưởng nhóm / Core OS & IPC Integrator |
| **2** | [Điền Họ và Tên thành viên 2] | [Điền MSSV] | Khối Hiển thị (Display) & Khối Media |
| **3** | [Điền Họ và Tên thành viên 3] | [Điền MSSV] | Khối Đầu vào (Input & ISR) & Testing |

*(Ghi chú: Thay đổi vai trò phân công cho phù hợp với thực tế nhóm của bạn)*

---

## ⚙️ 3. Dòng chảy dữ liệu (Data Flow) và Kiến trúc thư mục

Dự án được thiết kế theo mô hình **Component-Based Architecture**, tách biệt hoàn toàn Tầng trừu tượng phần cứng (HAL) và Logic điều khiển của FreeRTOS. Điều này giúp mã nguồn không bị lộn xộn (Spaghetti code) và dễ dàng mở rộng.

### Cấu trúc thư mục cốt lõi
* 📁 `lib/`: Chứa các Class C++ giao tiếp trực tiếp với phần cứng vật lý (Màn hình, Nút bấm, Thẻ SD). **Tuyệt đối không chứa thư viện FreeRTOS tại đây.**
* 📁 `include/`: Chứa các cấu hình dùng chung (`config.h`, định nghĩa chân GPIO) và các `struct` định nghĩa gói tin giao tiếp (`events.h`).
* 📁 `src/`: Chứa entry point `main.cpp` và thư mục `tasks/` – nơi khởi tạo môi trường đa nhiệm và các vòng lặp tác vụ của FreeRTOS.

### Giải thích Dòng chảy dữ liệu (Data Flow) trong hệ thống

Dòng chảy dữ liệu tuân thủ nguyên tắc **Event-Driven (Hướng sự kiện)**, di chuyển từ phần cứng lên đến logic xử lý như sau:

1.  **Kích hoạt phần cứng (Hardware Trigger):** Khi người dùng thao tác trên Joystick hoặc nút bấm, tín hiệu điện áp thay đổi.
2.  **Xử lý Ngắt (Interrupt Service Routine - ISR):** * Chân GPIO phát hiện thay đổi trạng thái và lập tức gọi hàm ISR nằm trong `src/tasks/input_task.cpp`.
    * Thuật toán chống dội phím (Debounce) bằng timer vật lý được kích hoạt.
    * Hàm ISR **không** xử lý logic đồ họa. Nó chỉ đóng gói thông tin (ví dụ: `BTN_A_PRESSED`) thành một struct `InputEvent` (định nghĩa tại `include/events.h`).
3.  **Giao tiếp qua Hàng đợi (Message Queue):** Hàm ISR sử dụng API `xQueueSendFromISR()` để ném sự kiện vừa đóng gói vào một Hàng đợi chung của hệ thống, sau đó lập tức thoát ra để nhả CPU.
4.  **Đánh thức Tác vụ (Task Wake-up & Processing):** * `InputTask` (đang ở trạng thái Blocked để tiết kiệm CPU) nhận được dữ liệu từ Queue, lập tức thức dậy (Ready -> Running).
    * Nó phân tích sự kiện và gửi lệnh tương ứng qua một Queue khác tới `DisplayTask` hoặc `SystemTask`.
5.  **Bảo vệ Tài nguyên dùng chung (Mutex Locking):** * Nếu `DisplayTask` cần vẽ màn hình mới (dùng SPI Bus) và `SystemTask` cần đọc file từ thẻ nhớ (cũng dùng SPI Bus), cả hai phải xin cấp phép từ một **Mutex**.
    * Task nào giữ "chìa khóa" Mutex sẽ được quyền sử dụng Bus vật lý. Task còn lại phải chờ (Blocked) cho đến khi bus được giải phóng, triệt tiêu hoàn toàn nguy cơ rác dữ liệu do tranh chấp.
6.  **Thực thi HAL:** Các Task gọi ngược xuống các phương thức C++ thuần túy trong thư mục `lib/` (như `DisplayManager.drawFrame()`) để ra lệnh cho phần cứng thực thi kết quả cuối cùng.

Kiến trúc này đảm bảo thiết bị phản hồi mượt mà với độ trễ cực thấp (low-latency), âm thanh và hình ảnh chạy song song mà không xảy ra hiện tượng Crash hay Stack Overflow.