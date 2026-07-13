# AGENTS.md - Ghi nhớ dự án AutoPhoto

## Dự án là gì

AutoPhoto là ứng dụng desktop xử lý ảnh hàng loạt. Tính năng chính: che mặt tự động (face blur), ghép watermark timestamp, nén ảnh, xuất nhiều định dạng. Cung cấp cả giao diện GUI (Qt Quick/QML) lẫn chế độ CLI headless (`--mode auto`).

Phiên bản hiện tại: 0.1.0.

## Tính năng hiện có

### Face Detection & Privacy Blur
- Nhận diện mặt bằng **YuNet** (ONNX model `models/face_detection_yunet_2023mar.onnx`) chạy trên OpenCV DNN.
- **Haar cascade fallback**: `haarcascade_frontalface_default.xml` + `haarcascade_profileface.xml` (bao gồm mirror).
- 2 chế độ blur: **Gaussian** và **Pixelate**.
- Strength adjustable 1-100.
- Privacy mask oval rất mạnh (strength 100): expands vùng blur ra ngoài bbox mặt 45% ngang, 52% trên, 42% dưới, dùng ellipse mask feathered blend.

### False Positive Filters
- **Box Size Filter**: Loại bỏ detection có diện tích < 0.1% hoặc > 60% ảnh, aspect ratio ngoài 0.3-3.0.
- **Skin-color Filter**: Kiểm tra vùng HSV trong ROI, reject nếu skin ratio < 15%.
- **Aggregate skin-color validation**: Nếu avg skin ratio của tất cả face < 20% → reject ALL detections → loại bỏ false positive trên ảnh không có mặt.
- **Cascade Cross-Check**: Nếu YuNet detect 0 face → cascade faces cũng bị reject.
- **Face merge**: IoU > 0.25 hoặc overlap > 45% → merge boxes.

### Face Detection Cache
- `QCache<100>` lưu kết quả detect theo `(filePath, sensitivity)`. Tránh detect lại ảnh đã xử lý trong cùng session.

### Timestamp Watermark
- Hiển thị text tùy chỉnh (custom text) hoặc ngày giờ file (creation/modification time).
- 10 font: Arial, Courier New, Times New Roman, Comic Sans MS, Verdana, Impact, Georgia, Trebuchet MS, Tahoma, Lucida Console.
- 8 màu cơ bản + custom.
- 4 vị trí cố định (TopLeft, TopRight, BottomLeft, BottomRight) + Custom X/Y.
- Date format linh hoạt: 10 kiểu ngày + 2 kiểu giờ.
- Font size tự scale theo resolution ảnh (`baseScale = max(1.0, rows/1000)`).
- Drop shadow đen确保 chữ đọc được trên nền sáng.
- **Chỉ có trên GUI** — CLI hiện chưa có flag `--timestamp*`.

### Image Compression
- Compression level 0-100%, hỗ trợ JPG/PNG/WEBP.
- Pipeline: ghi tạm JPG → `ImageCompressor::compress()` → xóa tạm.

### EXIF Auto-Rotate
- `readImageWithResolvedOrientation()` tự đọc EXIF orientation và xoay ảnh đúng hướng.
- Cover Flow hiển thị ảnh đã resolved orientation + blur.
- Thumbnail nhỏ bên dưới Cover Flow là ảnh gốc, không blur, dùng raw pixel orientation.

### Pipeline đa luồng (BatchProcessor)
- **3-stage producer-consumer pipeline**: Reader → Detector → Writer, chạy trên `std::thread` riêng.
- `BoundedBuffer<T>` — lock-free bounded queue với condition variables.
- Buffer size tự điều chỉnh theo RAM: 4 (>4GB), 2 (default), 1 (<2GB).
- Hỗ trợ Pause/Resume/Stop bằng `std::atomic_bool`.
- Progress callback qua `QMetaObject::invokeMethod` để update UI từ writer thread.

### CLI Headless Mode
- `--mode auto` bỏ qua GUI, chạy batch job trong terminal.
- Các flag hiện có:
  ```
  --mode <gui|auto>            Run mode
  --input <folder>             Input image folder (headless)
  --output <folder>            Output folder (headless)
  --rename-pattern <pattern>   Output filename pattern
  --no-blur-faces              Disable face blur
  --no-rotate                  Disable EXIF auto-rotation
  --blur-mode <gaussian|pixelate>
  --strength <1-100>           Blur strength
  --detection-sensitivity <0-100>  Face detection sensitivity
  --no-size-filter             Disable box size filter
  --no-skin-filter             Disable skin-color filter
  --no-cascade-cross-check     Disable cascade cross-check filter
  --compression <0-100>        Image compression level
  --output-format <jpg|png|webp>  Output format
  ```
- **Lưu ý**: Timestamp watermark hiện chỉ có trên GUI, chưa có flag CLI.

### Giao diện GUI
- Qt Quick Controls 2, Material Design Light (accent Teal, primary BlueGrey).
- **Cover Flow** (PathView): nửa cung, 7 items visible, scale/opacity/rotate theo path. Hiển thị preview processed ngay trên ảnh.
- **ToolPanel** (panel phải 344px): Images, Output, Rotate, Face Blur (mode + strength + sensitivity + 3 filters), Image Compression, Timestamp Watermark.
- **Bottom bar**: Start/Pause/Resume/Stop + progress bar + status text + worker count.
- Header: tiêu đề "AutoPhoto" + image count, không có nút Source/Output dư.
- Bảng màu: nền `#f7f8fa`, chữ `#111827`, accent `#2563eb`, success `#0f766e`.

### SSH Tunnel Controller
- Placeholder trong UI (`SshTunnelController`). Chưa kết nối thật. Serve như reminder cho feature tương lai.

## Kiến trúc

```
src/
├── main.cpp                    Entry point: CLI parser + QML engine setup
├── ImageProcessor.h/.cpp       Xử lý 1 ảnh: EXIF → detect → filters → blur → timestamp → compress → export
├── ImageProcessorInternal.h    Decls nội bộ cho ImageProcessor (model paths, detect, filter, blur)
├── BatchProcessor.h/.cpp       Pipeline đa luồng 3 stage (Reader → Detector → Writer)
├── BoundedBuffer.h             Lock-free bounded queue template
├── ImageListModel.h/.cpp       QAbstractListModel cho QML (load folder, roles: FilePath/Url/Name/Status)
├── PreviewController.h/.cpp    Controller preview 1 ảnh (debounce, regenerateFast + regenerate)
├── ImageCompressor.h/.cpp      Nén ảnh JPG/PNG/WEBP theo level
├── ImageOrientation.h/.cpp     Đọc EXIF orientation, xoay ảnh
├── SshTunnelController.h/.cpp  SSH tunnel placeholder
└── platform/
    ├── PlatformInterface.h     Abstract interface (loadImage, saveImage, screenScale, cacheDir, modelDir)
    ├── DesktopPlatform.h/.cpp  Implementation cho desktop
    └── PlatformFactory.h/.cpp  Factory pattern cho future Android/iOS

qml/
├── Main.qml                    Layout chính
└── components/
    ├── CoverFlowView.qml       Cover Flow preview
    └── ToolPanel.qml           Panel cài đặt bên phải

models/
└── face_detection_yunet_2023mar.onnx   YuNet face detection model

scripts/
├── build-macos.sh              Build + package macOS (.app + .dmg)
└── build-windows.ps1           Build + package Windows (portable folder + ZIP)

packaging/
└── windows/AutoPhoto.iss.in    Inno Setup template cho Windows installer
```

## Quy ước phát triển

- Giữ kiến trúc lai GUI + CLI.
- Ưu tiên sửa/mở rộng code hiện có thay vì thay stack.
- Giao diện là công cụ xử lý hàng loạt, không phải landing page/marketing.
- Style: Material Design sáng, nền trắng/xám nhạt, chữ tối, accent teal.
- Start/Pause/Stop đặt ở khu vực dưới Cover Flow; panel phải không có block Run riêng.
- Không thêm lại UI Rotate thủ công 0/90/180/270 trừ khi người dùng yêu cầu rõ ràng.
- Luồng đọc ảnh dùng `readImageWithResolvedOrientation()`.
- Cover Flow là khu vực preview kết quả xử lý: resolved orientation + blur hiển thị ở đó.
- Thumbnail nhỏ dưới Cover Flow là ảnh gốc/origin: không blur, dùng raw pixel orientation.
- Không xuất thêm file test có prefix `orientfix*`.
- Khi cần test export tạm, ưu tiên output vào `/tmp/...`.
- Không tuyên bố ứng dụng hoàn chỉnh nếu chưa build, chạy GUI smoke test, chạy CLI smoke test và kiểm tra workflow ảnh thật.

## Lệnh hữu ích

```bash
# Build
cmake -S . -B build -DAUTOPHOTO_ENABLE_OPENCV=ON
cmake --build build --target autophoto
cmake --build build --target autophoto_qmllint

# Chạy GUI
./build/autophoto

# Chạy CLI headless
./build/autophoto --mode auto --input /path/to/images --output /path/to/output
./build/autophoto --mode auto --input /path --output /out --detection-sensitivity 50 --compression 50 --output-format webp

# Build macOS (produces .app + .dmg)
bash scripts/build-macos.sh

# Build Windows (produces portable folder + ZIP)
.\scripts\build-windows.ps1 -QtPrefix "C:\Qt\6.x.x\msvc2019_64" -OpenCvDir "C:\opencv\build"
```

## Gói runtime Qt/QML trên Ubuntu/WSLg

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

## macOS Packaging Notes

### Bundle Structure

```
AutoPhoto.app/Contents/
├── MacOS/autophoto          ← executable
├── Resources/models/        ← YuNet ONNX model
├── Frameworks/              ← bundled dylibs (Qt, OpenCV, GCC runtime)
└── Info.plist
```

### applicationDirPath() trên macOS

Trả về `AutoPhoto.app/Contents/MacOS/` (thư mục chứa executable, KHÔNG phải bundle root).

Để tìm resources trong `Contents/Resources/`:
```cpp
// ĐÚNG
QDir(QCoreApplication::applicationDirPath()).filePath("../Resources/models")
// SAI — tìm vào Contents/MacOS/models/
QDir(QCoreApplication::applicationDirPath()).filePath("models")
```

### Model Search Pattern (nên dùng)

```cpp
const QStringList candidates = {
    QDir(appDir).filePath("models"),                    // dev/build
    QDir(appDir).filePath("../Resources/models"),       // macOS .app
    QDir(appDir).filePath("../models"),                 // fallback
    QDir(appDir).filePath("../../models"),              // flat layout
    QStringLiteral("models"),                           // CWD-relative
};
```

### Code Signing

- Build script (`build-macos.sh`) thực hiện `codesign --force --deep --sign -` (ad-hoc) ở bước cuối.
- Nếu遇见 lỗi "phần mềm bị đóng đột ngột" / "không có chữ ký": thường là do mã hóa thay đổi sau khi macdeployqt đã seal signature → cần re-sign lại bundle. Script hiện đã xử lý bước này.
- Sau khi build xong, chạy `codesign --verify --deep --strict --verbose=2 AutoPhoto.app` để kiểm tra.

### Deployment Order (quan trọng)

1. `cmake -S . -B build` (configure)
2. `cmake --build build` (build → produces .app)
3. `cmake --install build` (install vào dist/)
4. `macdeployqt` (bundle Qt frameworks)
5. `dylibbundler` (bundle OpenCV + GCC runtime dylibs)
6. `codesign --force --deep --sign -` (re-sign bundle)
7. `cpack -G DragNDrop` (create .dmg)

### Known Issue: macOS Gatekeeper

Khi distribute .app自 build (không phải từ App Store hoặc Developer ID signed), macOS có thể hiển thị cảnh báo "phần mềm bị đóng đột ngột" hoặc "không thể mở vì không xác minh được nhà phát triển". Giải pháp: System Settings → Privacy & Security → cho phép chạy phần mềm. Script build đã tự động re-sign ad-hoc để giảm thiểu vấn đề này.
