# AGENTS.md - Ghi nhớ dự án AutoPhoto

## Trạng thái dự án

AutoPhoto là ứng dụng desktop xử lý ảnh hàng loạt. Dự án đã có GUI, CLI/headless mode, xử lý ảnh bằng OpenCV và nhận diện mặt bằng YuNet.

### Tính năng đã hoàn thành

- Build và mở giao diện Qt Quick trên Linux/WSLg.
- Tải thư mục ảnh nguồn vào giao diện Cover Flow dạng nửa cung.
- **Face Detection Sensitivity Slider**: người dùng điều chỉnh ngưỡng YuNet từ 0%-100% (map sang float 0.00-1.00).
- **False Positive Filters**: 3 bộ lọc (size filter, skin-color filter, cascade cross-check) với UI toggles.
- **Aggregate skin-color validation**: nếu avg skin ratio < 20% → reject ALL detections → loại bỏ false positive trên ảnh không có mặt.
- Cascade cross-check mặc định BẬT: nếu YuNet detect 0 face → cascade faces cũng bị reject.
- **Image Compression Slider**: 0%-100%, hỗ trợ JPG/PNG/WEBP, giảm ≥50% dung lượng.
- Che mặt bằng privacy mask oval rất mạnh (strength 100).
- Tự động chọn hướng hiển thị khi preview và export ảnh (EXIF orientation handling).
- Dùng Qt Quick Controls 2 Material Design sáng.
- Panel phải có đầy đủ: Images, Output, Face Blur (strength + sensitivity + filters), Image Compression.
- Header trên cùng đã bỏ hai nút Source/Output dư; chọn source/output chỉ còn trong panel phải.
- Cung cấp SSH tunnel placeholder trong UI (chưa kết nối thật).
- Có scaffold đóng gói cho Windows và macOS.
- **Platform Strategy scaffold**: `PlatformInterface` abstract + `DesktopPlatform` + `PlatformFactory` cho tương lai Android/iOS.

### Bug đã fix

- **False positive on non-face images**: `back.jpg` từ 2-5 FP → 0 FP bằng aggregate skin-color validation.
- **Slow preview/CoverFlow**: thêm `regenerateFast()` (thumbnail downscaled nhanh), CoverFlow hiển thị ảnh gốc ngay lập tức.
- **ToolPanel layout broken**: thay ScrollView bằng Flickable, ScrollBar riêng không overlap content.

### Vấn đề đang tồn tại

- **CPU/RAM usage cao**: mỗi lần chuyển ảnh, YuNet model load + face detection chạy full pipeline gây lag, CPU spike, RAM consumption cao. Cần tối ưu ở PLAN.md giai đoạn mới.
- **ScrollBar menu phải**: vẫn còn đè lên cạnh bên trái của menu, cần fix layout lại cho đẹp hơn.
- Giao diện cần được kiểm tra thủ công trên Windows.
- Packaging `.exe` trên Windows và `.app` trên macOS phải được test trên hệ điềuaméliorer đích.
- SSH tunnel chỉ là placeholder cho tới khi có IP tĩnh/server thật.

## Kiến trúc

- Xử lý ảnh lõi: C++17 và OpenCV.
- Giao diện: Qt Quick/QML với Qt Quick Controls 2 Material.
- Build và packaging: CMake.
- Chế độ CLI/headless: `--mode auto`.
- Model YuNet nằm tại `models/face_detection_yunet_2023mar.onnx`.
- **Platform Strategy**: `src/platform/PlatformInterface.h` (abstract), `DesktopPlatform.cpp`, `PlatformFactory.cpp`.
- **ImageCompressor**: `src/ImageCompressor.h/.cpp` - nén ảnh JPG/PNG/WEBP.
- **ProcessingOptions** struct chứa tất cả tùy chọn: blurFaces, blurMode, strength, detectionSensitivity, sizeFilterEnabled, skinColorFilterEnabled, cascadeCrossCheckEnabled, compressionLevel, outputFormat.

## Quy ước phát triển

- Giữ kiến trúc lai GUI + CLI.
- Ưu tiên sửa/mở rộng code Qt/QML/C++ hiện có thay vì thay stack.
- Giao diện là công cụ xử lý hàng loạt, không làm kiểu landing page/marketing.
- Style hiện dùng Material Design sáng: nền trắng/xám nhạt, chữ tối, accent teal.
- Start/Pause/Stop chỉ đặt ở khu vực dưới Cover Flow; panel phải không có block Run riêng.
- Không thêm lại UI Rotate thủ công 0/90/180/270 trừ khi người dùng yêu cầu rõ ràng.
- Luồng đọc ảnh dùng `readImageWithResolvedOrientation()`.
- Cover Flow là khu vực preview kết quả xử lý: resolved orientation và blur phải hiển thị ở đó.
- Thumbnail nhỏ dưới Cover Flow là ảnh gốc/origin: không blur và dùng raw pixel orientation.
- Không xuất thêm file test có prefix `orientfix*`.
- Khi cần test export tạm, ưu tiên output vào `/tmp/...` thay vì ghi thêm vào `data_test/output`.
- Không tuyên bố ứng dụng hoàn chỉnh nếu chưa build, chạy GUI smoke test, chạy CLI smoke test và kiểm tra workflow ảnh thật.
- **Tối ưu hiệu năng là ưu tiên cao**: CPU/RAM usage phải ở mức chấp nhận được cho người dùng cuối.

## Lệnh hữu ích

```bash
cmake -S . -B build -DAUTOPHOTO_ENABLE_OPENCV=ON
cmake --build build --target autophoto
cmake --build build --target autophoto_qmllint
./build/autophoto
./build/autophoto --mode auto --input /path/to/images --output /path/to/output
./build/autophoto --mode auto --input /path --output /out --detection-sensitivity 50 --compression 50 --output-format webp
```

## CLI options hiện có

```
--mode <gui|auto>           Run mode
--input <folder>            Input image folder (headless)
--output <folder>           Output folder (headless)
--rename-pattern <pattern>  Output filename pattern
--no-blur-faces             Disable face blur
--blur-mode <gaussian|pixelate>
--strength <1-100>          Blur strength
--detection-sensitivity <0-100>  Face detection sensitivity
--no-size-filter            Disable box size filter
--no-skin-filter            Disable skin-color filter
--cascade-cross-check       Enable cascade cross-check
--compression <0-100>       Image compression level
--output-format <jpg|png|webp>  Output format
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

## macOS Qt .app Bundle Packaging

### .app Bundle Directory Structure

```
AutoPhoto.app/
├── Contents/
│   ├── MacOS/          ← executable lives here
│   │   └── autophoto
│   ├── Resources/      ← app resources (models, icons, configs)
│   │   └── models/
│   ├── Frameworks/     ← bundled dylibs (Qt frameworks, OpenCV, etc.)
│   └── Info.plist
```

### applicationDirPath() on macOS

`QCoreApplication::applicationDirPath()` returns `AutoPhoto.app/Contents/MacOS/` (the directory containing the executable, NOT the bundle root).

To find resources in `Contents/Resources/`, use relative path `../Resources/` from applicationDirPath:

```cpp
// Correct — finds Contents/Resources/models/
QDir(QCoreApplication::applicationDirPath()).filePath("../Resources/models")

// Wrong — finds Contents/MacOS/models/ (not a standard location)
QDir(QCoreApplication::applicationDirPath()).filePath("models")
```

### CMake Install Paths for macOS Bundle

When installing resources into a macOS .app bundle, use:

```cmake
# Install models to the standard Resources location
install(FILES "${MODEL_FILE}" DESTINATION "AutoPhoto.app/Contents/Resources/models")
```

NOT `AutoPhoto.app/Contents/MacOS/models` — that puts resources next to the binary, which is non-standard and breaks when macdeployqt restructures the bundle.

### Deployment Order (Critical)

Deploy steps MUST run AFTER the build produces the .app bundle. Common mistake: placing deploy steps before configure/build in CI scripts.

Correct order:
1. `cmake -S . -B build` (configure)
2. `cmake --build build` (build → produces .app)
3. `macdeployqt build/AutoPhoto.app` (bundle Qt frameworks)
4. `dylibbundler` (bundle third-party dylibs like OpenCV)
5. `cpack -G DragNDrop` (create .dmg)

### dylibbundler for OpenCV

```bash
EXEC_FILE=$(find "$APP_DIR/Contents/MacOS" -type f -perm +111 | head -n 1)
mkdir -p "$APP_DIR/Contents/Frameworks/"
dylibbundler -od -b \
  -x "$EXEC_FILE" \
  -d "$APP_DIR/Contents/Frameworks/" \
  -p @executable_path/../Frameworks/
```

### Model Search Pattern (Recommended)

When searching for bundled resources, always include both the executable-adjacent path and the Resources path:

```cpp
const QStringList candidates = {
    QDir(appDir).filePath("models"),                    // dev/build: next to executable
    QDir(appDir).filePath("../Resources/models"),       // macOS .app: Contents/Resources/
    QDir(appDir).filePath("../models"),                 // macOS .app: Contents/models (fallback)
    QDir(appDir).filePath("../../models"),              // flat layout
    QStringLiteral("models"),                           // CWD-relative
};
```
