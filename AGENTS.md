# AGENTS.md - Ghi nhớ dự án AutoPhoto

## Trạng thái dự án

AutoPhoto là ứng dụng desktop xử lý ảnh hàng loạt. Dự án hiện là bản chức năng đang phát triển, đã có GUI, CLI/headless mode, xử lý ảnh bằng OpenCV và nhận diện mặt bằng YuNet.

Ứng dụng hiện có thể:
- Build và mở giao diện Qt Quick trên Linux/WSLg.
- Tải thư mục ảnh nguồn vào giao diện Cover Flow dạng nửa cung.
- Tạo preview xử lý cho ảnh đang chọn ở vùng Cover Flow trung tâm.
- Hiển thị thumbnail gốc/origin ở vùng dưới Cover Flow.
- Chạy batch export từ GUI và CLI/headless mode.
- Làm mờ mặt bằng YuNet ONNX model kết hợp Haar cascade/profile fallback.
- Che mặt bằng privacy mask oval rất mạnh, gần kiểu mẫu `data_test/example/half_face.jpg`.
- Tự động chọn hướng hiển thị khi preview và export ảnh, nhưng không tin mù quáng EXIF orientation nếu metadata làm ảnh đang đúng bị xoay sai.
- Xuất đúng hướng cho ảnh `data_test/image_source/20260416_102356.jpg`; file `data_test/output/autophoto_0002.jpg` đã được overwrite bằng output đúng `4080x3060`.
- Dùng Qt Quick Controls 2 Material Design sáng.
- Panel phải đã bỏ block Run và bỏ Rotate thủ công.
- Header trên cùng đã bỏ hai nút Source/Output dư; chọn source/output chỉ còn trong panel phải.
- Cung cấp SSH tunnel placeholder trong UI. Placeholder này chỉ giữ cấu hình và trạng thái giả lập, chưa kết nối SSH thật.
- Có scaffold đóng gói cho Windows và macOS.

Các điểm cần tiếp tục kiểm thử thực tế:
- Giao diện cần được kiểm tra thủ công trên Windows, nhất là Cover Flow và Material style.
- Detector face blur hiện đang quá nhạy: YuNet threshold `0.35` giúp bắt mặt partial nhưng gây false positive trên vùng không có mặt. Xem `PLAN.md` để xử lý tiếp.
- Packaging `.exe` trên Windows và `.app` trên macOS phải được test trên hệ điều hành đích.
- Batch nhiều ảnh lớn cần được kiểm tra hiệu năng và bộ nhớ.
- SSH tunnel chỉ là placeholder cho tới khi có IP tĩnh/server thật.

## Kiến trúc

- Xử lý ảnh lõi: C++17 và OpenCV.
- Giao diện: Qt Quick/QML với Qt Quick Controls 2 Material.
- Build và packaging: CMake.
- Chế độ CLI/headless: `--mode auto`.
- Model YuNet nằm tại `models/face_detection_yunet_2023mar.onnx` và được CMake copy vào cạnh executable trong `build/models`.

## Quy ước phát triển

- Giữ kiến trúc lai GUI + CLI.
- Ưu tiên sửa/mở rộng code Qt/QML/C++ hiện có thay vì thay stack.
- Giao diện là công cụ xử lý hàng loạt, không làm kiểu landing page/marketing.
- Style hiện dùng Material Design sáng: nền trắng/xám nhạt, chữ tối, accent teal.
- Start/Pause/Stop chỉ đặt ở khu vực dưới Cover Flow; panel phải không có block Run riêng.
- Không thêm lại UI Rotate thủ công 0/90/180/270 trừ khi người dùng yêu cầu rõ ràng.
- Luồng đọc ảnh dùng `readImageWithResolvedOrientation()`: ảnh landscape có EXIF yêu cầu xoay 90/270 sẽ giữ raw pixel orientation để tránh lỗi metadata kiểu `20260416_102356.jpg`.
- Cover Flow là khu vực preview kết quả xử lý: resolved orientation và blur phải hiển thị ở đó.
- Thumbnail nhỏ dưới Cover Flow là ảnh gốc/origin: không blur và dùng raw pixel orientation để khớp với ảnh nguồn.
- Không xuất thêm file test có prefix `orientfix*`.
- Khi cần test export tạm, ưu tiên output vào `/tmp/...` thay vì ghi thêm vào `data_test/output`.
- Không tuyên bố ứng dụng hoàn chỉnh nếu chưa build, chạy GUI smoke test, chạy CLI smoke test và kiểm tra workflow ảnh thật.

## Lệnh hữu ích

```bash
cmake -S . -B build -DAUTOPHOTO_ENABLE_OPENCV=ON
cmake --build build --target autophoto
cmake --build build --target autophoto_qmllint
./build/autophoto
./build/autophoto --mode auto --input /path/to/images --output /path/to/output
```

## Gói runtime Qt/QML thường cần trên Ubuntu/WSLg

```bash
sudo apt install \
  qml6-module-qtqml \
  qml6-module-qtqml-workerscript \
  qml6-module-qtquick \
  qml6-module-qtquick-controls \
  qml6-module-qtquick-templates \
  qml6-module-qtquick-dialogs \
  qml6-module-qtquick-layouts \
  qml6-module-qtquick-window
```
