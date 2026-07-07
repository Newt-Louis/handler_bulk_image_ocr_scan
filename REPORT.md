# Báo cáo kỹ thuật tổng quan dự án AutoPhoto

## 1. Giới thiệu dự án
**AutoPhoto** là một ứng dụng Desktop chuyên dụng cho việc xử lý hàng loạt hình ảnh, với các chức năng chính bao gồm che mờ khuôn mặt tự động (bảo vệ quyền riêng tư), nén ảnh dung lượng thấp và đóng dấu bản quyền (Timestamp Watermark). Dự án được thiết kế đa nền tảng và hỗ trợ cả chế độ Giao diện đồ hoạ (GUI) lẫn chế độ dòng lệnh (CLI).

## 2. Công nghệ sử dụng
- **Core / Backend**: C++17
- **Giao diện (UI)**: Qt 6 (Qt Quick, QML, Qt Quick Controls 2 Material Design)
- **Xử lý ảnh & AI**: OpenCV 4.x
- **Mô hình nhận diện khuôn mặt**: YuNet (DNN model `face_detection_yunet_2023mar.onnx`)
- **Đóng gói và Build**: CMake, MacDeployQt (macOS), CPack.

## 3. Kiến trúc xử lý đa luồng (Multi-threading & Pipeline)
Để đảm bảo UI không bị đứng (freeze) khi xử lý hàng nghìn bức ảnh, hệ thống xử lý hàng loạt (`BatchProcessor.cpp`) áp dụng kiến trúc **Pipeline 3 giai đoạn** sử dụng hàng đợi (Queue) có giới hạn (`BoundedBuffer`):

1. **Reader Stage (Đọc ảnh)**: Đọc ảnh từ ổ cứng, xử lý EXIF Orientation (chống lật ảnh tự động) -> đẩy vào `readBuffer`.
2. **Detector Stage (Nhận diện)**: Lấy ảnh từ `readBuffer`, đưa qua mô hình YuNet để phát hiện khuôn mặt, lọc False Positive (sai lệch) -> đẩy vào `detectBuffer`.
3. **Writer Stage (Ghi ảnh)**: Lấy dữ liệu từ `detectBuffer`, tiến hành làm mờ (Blur), chèn Watermark, nén (Compress) và ghi ra ổ cứng. 

**Tối ưu hoá RAM & CPU (Nhiệm vụ 11):**
- **Theo dõi RAM (Memory Monitoring)**: Ứng dụng tự động theo dõi dung lượng RAM khả dụng của máy tính mỗi chu kỳ 10 ảnh thông qua API hệ thống (`GlobalMemoryStatusEx` trên Windows, `sysctl` trên macOS, `sysconf` trên Linux).
- **Dynamic Buffer Size**: 
  - Dung lượng RAM khả dụng > 4GB: Sử dụng Buffer size = 4 (tăng tốc độ nạp ảnh lên RAM).
  - Dung lượng RAM khả dụng < 2GB: Sử dụng Buffer size = 1 (tránh tràn RAM gây crash).
  - Mặc định: Buffer size = 2.
- **Tối ưu OpenCV Threads**: Giới hạn `cv::setNumThreads(2)` thay vì 4, để giảm tranh chấp CPU (CPU Contention) khi chạy song song 3 workers trong Pipeline.
- **Tối ưu UI Preview**: Giao diện dùng `QThreadPool` thay cho việc gọi `QThread::create()` liên tục khi người dùng kéo thanh trượt (slider). Nhờ đó, tài nguyên luồng được tái sử dụng, giúp giảm đáng kể mức ngốn CPU và RAM.

## 4. Đặc tả Giao diện (UI/UX)
Sử dụng **Material Design** với 2 khu vực chính:
- **Main View**: Sử dụng `PathView` (hiệu ứng Cover Flow) hiển thị thumbnail các ảnh gốc (bên dưới) và ảnh sau khi preview hiệu ứng (bên trên). 
- **Tool Panel (Thanh công cụ bên phải)**:
  - Chọn thư mục nguồn / đích.
  - **Face Blur**: Tuỳ chọn chế độ (Gaussian/Pixelate), cường độ (Strength 1-100), thanh trượt độ nhạy YuNet (Sensitivity).
  - **False Positive Filters**: Bộ lọc Box Size, Skin Color, và Cascade Cross-check.
  - **Image Compression**: Nén ảnh giảm tới 90% dung lượng, hỗ trợ định dạng JPG, PNG, WEBP.
  - **Timestamp Watermark**: Đóng dấu thời gian ở 4 góc hoặc vị trí tuỳ chỉnh (X, Y) bằng text đơn sắc.
  - **Tiến trình**: ProgressBar hiển thị phần trăm (thời gian thực), số lượng ảnh Success / Failed.

## 5. Các tính năng đặc trưng và cách thực hiện
1. **Bộ lọc loại bỏ nhận diện sai (False Positive Filters)**:
   - *Vấn đề*: YuNet có thể nhận diện sai các chi tiết giống khuôn mặt trên áo, chữ viết.
   - *Giải pháp*:
     - **Skin Color Filter**: Chuyển đổi vùng nhận diện sang dải màu YCrCb để phân tích tỷ lệ màu da thật. Nếu tỷ lệ này < 20%, loại bỏ toàn bộ khuôn mặt trên ảnh đó (cho thấy ảnh có khả năng không chứa người).
     - **Cascade Cross-check**: Dùng mô hình Haar Cascade truyền thống để đối chiếu kết quả của YuNet. Nếu cả 2 đều không tìm thấy điểm chung (IoU > 15%), kết quả bị loại.

2. **Xử lý EXIF Orientation (Hướng ảnh)**:
   - Các ảnh chụp từ điện thoại thường lưu hướng lật trong EXIF thay vì xoay pixel thực tế. Ứng dụng tự động đọc thẻ EXIF (sử dụng libexif hoặc QImageReader) và xoay/lật ma trận pixel (`cv::Mat`) của OpenCV về hướng chuẩn trước khi đưa vào mô hình nhận diện, tránh việc YuNet không bắt được mặt vì ảnh đang nằm ngang.

3. **Timestamp Watermark**:
   - Tự động lấy thời gian hệ thống định dạng `yyyy-MM-dd HH:mm:ss`. Dùng hàm `cv::putText` (OpenCV) với thuật toán tính toán `cv::getTextSize` để neo text vào 4 góc (Top-Left, Top-Right, Bottom-Left, Bottom-Right) trừ đi lề 10px, hoặc toạ độ tĩnh (Custom).

4. **Nén ảnh (Compression)**:
   - Chạy trên một file trung gian (`QDir::tempPath()`). Sử dụng QImage để nén qua hệ thống mã hoá ảnh của Qt (ImageFormats) hoặc trực tiếp ghi thành chuẩn WEBP, tối đa hoá việc giảm dung lượng.
