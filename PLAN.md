# PLAN.md - Chiến lược & Kế hoạch thực hiện AutoPhoto

## Tổng quan

Dự án AutoPhoto cần 3 nhiệm vụ chính + 1 kiến trúc nền tảng:
1. **Nhiệm vụ 1**: Face Detection Sensitivity Slider (0%-100%)
2. **Nhiệm vụ 2**: False Positive Filters (size, skin-color, cascade cross-check)
3. **Nhiệm vụ 3**: Image Compression (0%-100%, JPG/PNG/WEBP)
4. **Kiến trúc Strategy**: Cross-platform scaffold (Desktop/Mobile)

---

## Nhiệm vụ 1: Face Detection Sensitivity Slider ✅

### Chi tiết thực hiện

- [x] **1.1** Thêm `float detectionSensitivity = 0.35f` vào `ProcessingOptions` trong `src/ImageProcessor.h`
- [x] **1.2** Sửa `detectWithYuNet()` nhận tham số `float scoreThreshold` thay vì hardcode `0.35f`
- [x] **1.3** Truyền `m_options.detectionSensitivity` xuống `detectWithYuNet()` trong `processFile()`
- [x] **1.4** Thêm Q_PROPERTY `detectionSensitivity` (float 0.0-1.0) vào `PreviewController.h`
- [x] **1.5** Thêm getter/setter/signal trong `PreviewController.cpp`
- [x] **1.6** Cập nhật `regenerate()` truyền `detectionSensitivity` vào `ProcessingOptions`
- [x] **1.7** Cập nhật `BatchProcessor.start()` nhận `float detectionSensitivity` parameter
- [x] **1.8** Thêm Slider "Detection Sensitivity" vào `ToolPanel.qml` (0-100, step 1)
- [x] **1.9** Hiển thị giá trị phần trăm (0%-100%) bên cạnh slider
- [x] **1.10** Bind ToolPanel → PreviewController → Main.qml → BatchProcessor
- [x] **1.11** Cập nhật cache hash bao gồm `detectionSensitivity`
- [x] **1.12** Thêm CLI option `--detection-sensitivity` cho headless mode

### Flow dữ liệu
```
ToolPanel.qml (Slider 0-100)
  → Main.qml (chuyển: value/100.0)
    → PreviewController.detectionSensitivity (float 0.0-1.0)
      → ProcessingOptions.detectionSensitivity
        → detectWithYuNet(image, modelPath, scoreThreshold)
```

---

## Nhiệm vụ 2: False Positive Filters ✅

### 2A. Bộ lọc kích thước box

- [x] **2A.1** Thêm hàm `filterByBoxSize()` trong `ImageProcessor.cpp`
- [x] **2A.2** Lọc box diện tích < 0.1% hoặc > 60% diện tích ảnh
- [x] **2A.3** Lọc box tỷ lệ W/H < 0.3 hoặc > 3.0
- [x] **2A.4** Áp dụng sau merge faces, trước apply blur

### 2B. Bộ lọc skin-color ratio

- [x] **2B.1** Thêm hàm `estimateSkinColorRatio()` (HSV, H: 0-50 + 170-180, S: 40-170, V: 80-255)
- [x] **2B.2** Nếu tỷ lệ pixel da < 15% trong ROI → bỏ qua
- [x] **2B.3** Chỉ áp dụng cho box có confidence thấp (detectionSensitivity < 0.60)

### 2C. Bộ lọc cascade cross-validation

- [x] **2C.1** Lưu `yuNetFaces` trước khi chạy cascade
- [x] **2C.2** Nếu cascade box không overlap YuNet box (IoU < 0.15) → bỏ qua
- [x] **2C.3** Hoạt động tốt nhất ở detectionSensitivity 35%-50%

### 2D. UI toggles

- [x] **2D.1** Switch "Size filter" (mặc định: bật)
- [x] **2D.2** Switch "Skin-color filter" (mặc định: bật)
- [x] **2D.3** Switch "Cascade cross-check" (mặc định: tắt)
- [x] **2D.4** Bind qua PreviewController → ProcessingOptions → ImageProcessor
- [x] **2D.5** CLI flags: `--no-size-filter`, `--no-skin-filter`, `--cascade-cross-check`

### Test results

| Config | back.jpg | half_face.jpg | 091122.jpg | 102356.jpg |
|--------|----------|---------------|------------|------------|
| 35% + cross-check | 1 FP | 1 face ✅ | 2 faces | 0 |
| 50% + cross-check | 1 FP | 1 face ✅ | - | - |
| 60% + cross-check | 1 FP | 7 over | - | - |
| No filters | 5 FP | 11 over | - | - |

**Ghi chú**: `back.jpg` là ảnh phong cảnh, YuNet vẫn detect 1 FP ở ngưỡng thấp (limit model). Người dùng cần tăng sensitivity để xử lý.

---

## Nhiệm vụ 3: Image Compression ✅

### 3A. Core compression engine

- [x] **3A.1** Tạo `ImageCompressor` class (`src/ImageCompressor.h`, `src/ImageCompressor.cpp`)
- [x] **3A.2** Hỗ trợ JPEG quality reduction (20-95)
- [x] **3A.3** Hỗ trợ resize/downscale (0.30x - 1.0x)
- [x] **3A.4** Hỗ trợ WEBP output (nén tốt hơn JPEG ~30%)
- [x] **3A.5** Hỗ trợ PNG output (lossless)
- [x] **3A.6** Thêm `compressionLevel` và `outputFormat` vào `ProcessingOptions`

### 3B. Compression mapping

| Level | JPEG Quality | Resize | Est. Reduction |
|-------|-------------|--------|----------------|
| 0% | 95 | 1.0x | 0% (gốc) |
| 25% | 85 | 1.0x | ~30% |
| 50% | 70 | 0.85x | ~55% |
| 75% | 50 | 0.70x | ~75% |
| 100% | 20 | 0.30x | ~90% |

### 3C. Integration

- [x] **3C.1** Compression step trong `ImageProcessor::processFile()` SAU blur
- [x] **3C.2** PreviewController: compression preview trong Cover Flow
- [x] **3C.3** BatchProcessor: truyền compression level xuống workers
- [x] **3C.4** Cache hash bao gồm compression level + output format

### 3D. UI

- [x] **3D.1** GroupBox "Image Compression" trong ToolPanel.qml
- [x] **3D.2** Slider 0-100%, hiển thị estimated reduction
- [x] **3D.3** ComboBox "Output Format" (JPG, PNG, WEBP)
- [x] **3D.4** Bind qua PreviewController → ProcessingOptions → ImageProcessor

### 3E. CLI

- [x] **3E.1** `--compression <0-100>` option
- [x] **3E.2** `--output-format <jpg|png|webp>` option
- [x] **3E.3** Tự động đổi extension file output theo format

### Test results

| Level | Format | 091122.jpg | 102356.jpg | Giảm |
|-------|--------|-----------|-----------|------|
| Original | JPG | 3.8M | 3.5M | - |
| 25% | JPG | 1.9M | 781K | ~50-78% |
| 50% | JPG | 854K | 321K | ~78-91% |
| 50% | WEBP | 423K | 101K | ~89-97% |

✅ Vượt yêu cầu giảm ≥50% dung lượng.

---

## Kiến trúc Strategy cho Cross-Platform ✅

### Scaffold đã tạo

- [x] **P1** `PlatformInterface` abstract class (loadImage, saveImage, screenScale, cacheDir, modelDir, hasOpenCV)
- [x] **P2** `DesktopPlatform` implementation (Qt-based)
- [x] **P3** `PlatformFactory` với `#ifdef Q_OS_ANDROID / Q_OS_IOS`
- [x] **P4** Thêm vào `CMakeLists.txt`

### Khi nào cần implement đầy đủ

- Android: thay Qt Quick bằng Android Views/Compose, dùng CameraX API
- iOS: thay Qt Quick bằng SwiftUI/UIKit, dùng ARKit/AVFoundation
- Core logic (ImageProcessor, BatchProcessor, ImageCompressor) giữ nguyên, chỉ wrap platform calls

---

## Thứ tự thực hiện đã hoàn thành

1. ✅ T1: Face Detection Sensitivity Slider
2. ✅ T2: False Positive Filters
3. ✅ T3: Image Compression
4. ✅ T4: Platform Strategy Scaffold

## Tổng kết

- Build thành công trên Linux
- QML lint pass (chỉ warnings, không errors)
- Tất cả features hoạt động qua CLI test
- GUI cần test thủ công trên desktop
