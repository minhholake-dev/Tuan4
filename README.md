# Tuần 04 — MQTT, JSON và điều khiển LED

Các bài thực hành gửi dữ liệu ADC qua MQTT, nhận và kiểm tra JSON, gửi dữ liệu giả lập và điều khiển LED bằng lệnh mạng.

## Danh mục và môi trường chạy

| Bài | Nội dung | Mã nguồn | Environment |
| --- | --- | --- | --- |
| 01 | Gửi hai giá trị ADC thô mỗi 2 giây | [Publisher](bai-tap/bai-01-mqtt-publisher/main.cpp) | `mqtt-publisher` |
| 02 | Gửi ADC và nhận JSON trên hai board | [Publisher](bai-tap/bai-02-mqtt-pub-sub/publisher/main.cpp) · [Subscriber](bai-tap/bai-02-mqtt-pub-sub/subscriber/main.cpp) | `mqtt-pubsub-publisher` · `mqtt-subscriber` |
| 03 | Dữ liệu giả lập và lệnh `0`/`1` điều khiển LED | [MQTT và LED](bai-tap/bai-03-mqtt-led/main.cpp) | `mqtt-led` |

`main` là môi trường mặc định, chạy cùng bài 03 qua [src/main.cpp](src/main.cpp). Chọn trực tiếp môi trường muốn chạy; không cần chép đè sketch.

```powershell
pio run -e mqtt-publisher
pio run -e mqtt-subscriber -t upload
pio device monitor -b 115200
```

Với bài 02, nạp publisher và subscriber lên hai board khác nhau. Mã client MQTT gồm vai trò và mã chip để tránh hai thiết bị ngắt kết nối của nhau. Các bài dùng chung [analog_publisher.h](include/analog_publisher.h) và [mqtt_runtime.h](include/mqtt_runtime.h) để giữ hành vi kết nối và định dạng dữ liệu nhất quán.

## Cấu hình riêng tư

Thông tin Wi-Fi, broker, tài khoản MQTT, topic và thông tin sinh viên nằm trong **`include/secrets.h`**. File này bị [Git bỏ qua](.gitignore); bản cục bộ giữ các cấu hình mạng đã có, đồng thời lưu cấu hình publisher cũ với tiền tố `kPreviousPublisher` để đối chiếu. Cặp publisher/subscriber hiện dùng cùng cấu hình hoạt động.

Khi sao chép dự án sang máy khác:

```powershell
Copy-Item -LiteralPath 'include/secrets.example.h' -Destination 'include/secrets.h'
```

Chỉ chạy lệnh sao chép khi chưa có file cục bộ để tránh ghi đè thiết lập của mình. Điền cấu hình vào file vừa tạo. [secrets.example.h](include/secrets.example.h) chỉ chứa giá trị trống và có thể đưa lên Git. Không thêm dữ liệu thật vào file mẫu. Mã sẽ bỏ qua việc kết nối nếu thiếu các trường bắt buộc.

- Publisher và subscriber cần cùng `kMqttHost`, `kMqttPort` và `kTelemetryTopic`.
- Bài 03 nhận lệnh ở `kLedCommandTopic`; payload phải chính xác một byte `0` hoặc `1`.
- `kMqttUsername` và `kMqttPassword` có thể để trống nếu broker phòng thực hành không yêu cầu đăng nhập.
- `kStudentName` và `kStudentId` là trường tùy chọn; để trống để không gửi thông tin cá nhân.
- Broker ở cổng 1883 với `WiFiClient` truyền dữ liệu **không mã hóa**; cấu hình này dành cho bài thực hành trong mạng tin cậy. Trước khi dùng qua Internet, cần chuyển sang TLS và xác minh chứng chỉ của broker. Đưa thông tin vào `.gitignore` chỉ ngăn file mới được Git theo dõi, không bảo vệ nội dung đã có trong lịch sử commit hoặc firmware.

## Board và kết nối

Board giữ là `esp32-c3-devkitc-02`, framework Arduino, nền tảng và thư viện được khai báo trong [platformio.ini](platformio.ini). Chân được gom trong [board_config.h](include/board_config.h):

| Thành phần | GPIO |
| --- | --- |
| Đầu vào analog cho kênh nhiệt độ | 0 |
| Đầu vào analog cho kênh độ ẩm | 1 |
| LED rời của bài 03 | 4 |

LED rời cần điện trở hạn dòng; cấu hình này không điều khiển LED RGB tích hợp của board. Nối chung GND và giới hạn điện áp analog trong **0–3,3 V**. Các chân cũ 22/34/35 không phù hợp ESP32-C3 nên đã được thay.

Bài 01 và publisher bài 02 gửi kết quả ADC 12 bit `0..4095`. Đây **chưa phải** nhiệt độ °C hay độ ẩm %. Bài 03 gửi giá trị giả lập. Trường `measurement` phân biệt `adc_raw` và `simulated`; subscriber in đúng loại dữ liệu. Muốn dùng cảm biến thật cần thay hàm đọc và hiệu chuẩn theo cảm biến.

## Định dạng và xử lý lỗi

Các chương trình thống nhất khóa JSON `temperature`, `humidity`, `measurement` và `uptime`. Subscriber kiểm tra độ dài tối đa 512 byte, kiểu trường, độ sâu JSON, loại phép đo và phạm vi dữ liệu trước khi sử dụng. Callback đọc payload theo độ dài được truyền vào, không coi dữ liệu MQTT là chuỗi đã có ký tự kết thúc.

Các lần thử lại Wi-Fi/MQTT có khoảng chờ và không dùng vòng lặp chờ kết nối vô hạn. TCP có giới hạn thời gian kết nối 1,5 giây; MQTT có thời gian chờ phản hồi 2 giây. Mỗi lần thử vẫn có thể chặn ngắn trong giới hạn của thư viện. Khi MQTT kết nối lại, chương trình gửi lại yêu cầu subscribe. `mqtt.subscribe()` xác nhận gửi yêu cầu; việc nhận dữ liệu thực tế còn cần broker cho phép topic và thiết bị publisher hoạt động.

Lịch gửi dùng phép trừ số nguyên không dấu để tiếp tục hoạt động khi `millis()` tràn. Các lỗi khai báo, cách gọi `connect()`/`connected()`, topic không khớp và trường JSON không khớp của bài gốc đã được sửa. Việc biên dịch không thay thế kiểm tra thực tế với board, broker và mạch nối.
