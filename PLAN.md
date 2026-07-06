# PLAN.md - Tối ưu hiệu năng & Feature toggles

## Bối cảnh

- Ứng dụng xử lý ảnh hàng loạt, target 200-500+ ảnh/lần
- Hiện tại: 4-5s/ảnh → 500 ảnh = 37 phút → không chấp nhận production
- Yêu cầu: < 1.5s/ảnh batch, preview instant
- Target hardware: Intel i5 11th gen, 16GB DDR4, no discrete GPU, HDD
- Yêu cầu: chạy nền mượt, không đơ/treo máy, RAM < 500MB

---

## Nhiệm vụ 8: Detection 640px + Intel Optimization ✅

- [v] **8.1** Giảm `maxDetectSide` từ 1200 → 640 trong `detectWithYuNet()`
- [v] **8.2** Bật `cv::useOptimized(true)` + `cv::setNumThreads(4)` trong ImageProcessor
- **Kết quả mong đợi**: inference ~0.6-0.8s thay vì 2.5-3s

---

## Nhiệm vụ 9: Feature Toggles (Enable/Disable cho từng tính năng) ✅

### Chi tiết thực hiện

#### 9A. ProcessingOptions mới

- [v] **9A.1** Thêm `bool rotateEnabled = true` vào `ProcessingOptions`
- [v] **9A.2** Thêm `bool blurFaces = true` (thay thế `blurFaces`)
- [v] **9A.3** Thêm `bool compressionEnabled = false` vào `ProcessingOptions`
- [v] **9A.4** Cập nhật `ImageProcessor::processFile()` check toggle trước khi chạy

#### 9B. ToolPanel UI

- [v] **9B.1** Thêm GroupBox "Rotate" với Switch toggle
- [v] **9B.2** GroupBox "Face Blur" hiện có → thêm Switch toggle ở đầu
- [v] **9B.3** GroupBox "Image Compression" hiện có → thêm Switch toggle ở đầu

#### 9C. Bindings

- [v] **9C.1** PreviewController: thêm Q_PROPERTY cho 3 toggles
- [v] **9C.2** Main.qml: bind toggles từ ToolPanel → PreviewController
- [v] **9C.3** BatchProcessor.start(): nhận 3 toggle params
- [v] **9C.4** CLI flags: `--no-rotate`, `--no-blur-faces`, `--compression 0`

---

## Nhiệm vụ 10: Producer-Consumer Batch Pipeline ✅

### Kiến trúc 3-stage đã implement

```
Stage 1 (Reader):    readImage(path[i])   → readBuffer (cap=1)
Stage 2 (Detector):  detectYuNet + cascade → detectBuffer (cap=1)
Stage 3 (Writer):    blur + compress + write → disk
```

3 threads chạy song song, data truyền qua `BoundedBuffer` capacity=1.

### Chi tiết đã implement

#### 10A. Pipeline infrastructure

- [v] **10A.1** `ReadData` + `DetectedData` structs trong BatchProcessor.cpp
- [v] **10A.2** `BoundedBuffer<T>` template (thread-safe queue)
- [v] **10A.3** `readerStage()`: đọc ảnh từ disk EXIF-aware → readBuffer
- [v] **10A.4** `detectorStage()`: YuNet + cascade + filters → detectBuffer
- [v] **10A.5** `writerStage()`: blur + compress + write → disk

#### 10B. Pipeline integration

- [v] **10B.1** `BatchProcessor::start()` dùng 3 std::thread song song
- [v] **10B.2** Detector gọi trực tiếp `detectWithYuNet()` + `detectWithCascade()` + filters
- [v] **10B.3** Writer gọi `applyBlur()` + `ImageCompressor::compress()`
- [v] **10B.4** Progress tracking: writer thread update GUI qua QMetaObject::invokeMethod

#### 10C. Memory management

- [v] **10C.1** `data->image.release()` sau khi writer ghi xong mỗi ảnh
- [v] **10C.2** Buffer capacity = 1 (tối thiểu RAM: ~34MB/ảnh × 2 buffer = ~68MB peak)
- [ ] **10C.3** Memory monitoring: log RAM usage mỗi N ảnh — CHƯA IMPLEMENT

#### 10D. CPU throttle & HDD optimization

- [v] **10D.1** Reader yield 5ms sau mỗi lần đọc (HDD friendly)
- [v] **10D.2** Detector yield 2ms sau mỗi lần detect
- [v] **10D.3** Pause/Resume: spin-wait 50ms giữa các items
- [v] **10D.4** Cancel: atomic flag checked ở mỗi stage

### Flow hiện tại

```
Thread Reader:
  for each image:
    [pause check] → readImage() → push readBuffer → yield 5ms

Thread Detector:
  while !done:
    pop readBuffer → [pause check] → detectYuNet → cascade → filters → push detectBuffer → yield 2ms

Thread Writer:
  while !done:
    pop detectBuffer → [pause check] → applyBlur → compress → write → release mat
    update progress via QMetaObject::invokeMethod
```

### Files đã sửa

| File | Thay đổi |
|------|----------|
| `src/BatchProcessor.h` | Bỏ QPointer, thêm std::thread members |
| `src/BatchProcessor.cpp` | Viết lại hoàn toàn: 3 stage functions + pipeline orchestration |
| `src/ImageProcessor.cpp` | Move functions ra khỏi anonymous namespace |
| `src/ImageProcessorInternal.h` | Thêm declarations cho cascade/filter/blur/readImage |

### Kết quả mong đợi

- **RAM**: ~68MB peak (buffer capacity=1 × 2 buffers × ~34MB/ảnh)
- **CPU**: 3 threads, yield CPU giữa iterations → không spike
- **HDD**: Reader yield 5ms → không block disk I/O
- **Background**: 3 threads chạy nền, GUI responsive qua QMetaObject::invokeMethod

---

## Nhiệm vụ 11 (tương lai): Memory Monitoring + CPU Throttle Fine-tune

- [ ] **11.1** Log RAM usage mỗi 10 ảnh (dùng `GlobalMemoryStatusEx` trên Windows, `sysctl` trên macOS)
- [ ] **11.2** Dynamic buffer size: tăng/giảm capacity dựa trên RAM usage
- [ ] **11.3** cv::setNumThreads(2) thay vì 4 để tránh chiếm hết cores
- [ ] **11.4** Preview controller: reuse thread pool thay vì tạo QThread::create() mỗi lần

---

## Thứ tự thực hiện

1. ~~**Nhiệm vụ 8**: Detection 640px + Intel opt~~ ✅
2. ~~**Nhiệm vụ 9**: Feature toggles~~ ✅
3. ~~**Nhiệm vụ 10**: Producer-consumer pipeline~~ ✅
4. **Nhiệm vụ 11**: Memory monitoring + CPU throttle fine-tune

---

## Lưu ý

- **Preview**: LUÔN hiển thị dựa trên features đang bật (không tắt preview)
- **Pipeline**: phải giữ progress tracking + cancel support
- **Backward compatible**: CLI options hiện có vẫn hoạt động
- **Test**: dùng `data_test/image_source/` (2 ảnh lớn) + `data_test/example/` (2 ảnh test FP)
- **Platform Interface**: chưa được wired vào main pipeline → cần tích hợp
- **Build**: MSVC toolchain trên máy này thiếu `type_traits` header → cần fix environment trước khi test
