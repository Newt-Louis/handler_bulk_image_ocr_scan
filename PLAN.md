# PLAN.md - Tối ưu hiệu năng & Feature toggles

## Bối cảnh

- Ứng dụng xử lý ảnh hàng loạt, target 200-500+ ảnh/lần
- Hiện tại: 4-5s/ảnh → 500 ảnh = 37 phút → không chấp nhận production
- Yêu cầu: < 1.5s/ảnh batch, preview instant

---

## Nhiệm vụ 8: Detection 640px + Intel Optimization ✅

- [x] **8.1** Giảm `maxDetectSide` từ 1200 → 640 trong `detectWithYuNet()`
- [x] **8.2** Bật `cv::useOptimized(true)` + `cv::setNumThreads(4)` trong ImageProcessor
- **Kết quả mong đợi**: inference ~0.6-0.8s thay vì 2.5-3s

---

## Nhiệm vụ 9: Feature Toggles (Enable/Disable cho từng tính năng)

### Yêu cầu

Mỗi tính năng batch có toggle riêng:
- **Rotate**: tự động xoay ảnh theo EXIF orientation
- **Face Blur**: detect mặt + blur/mask
- **Compression**: giảm dung lượng ảnh

Toggle là checkbox kiểu switch. Khi BẬT → hiển thị config (sliders, radio, etc.). Khi TẮT → config bị disabled/ẩn.

### Chi tiết thực hiện

#### 9A. ProcessingOptions mới

- [ ] **9A.1** Thêm `bool rotateEnabled = true` vào `ProcessingOptions`
- [ ] **9A.2** Thêm `bool blurFacesEnabled = true` (thay thế `blurFaces`)
- [ ] **9A.3** Thêm `bool compressionEnabled = false` vào `ProcessingOptions`
- [ ] **9A.4** Cập nhật `ImageProcessor::processFile()` check toggle trước khi chạy

#### 9B. ToolPanel UI

- [ ] **9B.1** Thêm GroupBox "Rotate" với Switch toggle
  - Khi bật: hiển thị label "Auto-rotate based on EXIF"
  - Khi tắt: label disabled, greyed out
- [ ] **9B.2** GroupBox "Face Blur" hiện có → thêm Switch toggle ở đầu
  - Khi tắt: toàn bộ Blur config (mode, strength, sensitivity, filters) disabled
- [ ] **9B.3** GroupBox "Image Compression" hiện có → thêm Switch toggle ở đầu
  - Khi tắt: slider + format ComboBox disabled

#### 9C. Bindings

- [ ] **9C.1** PreviewController: thêm Q_PROPERTY cho 3 toggles
- [ ] **9C.2** Main.qml: bind toggles từ ToolPanel → PreviewController
- [ ] **9C.3** BatchProcessor.start(): nhận 3 toggle params
- [ ] **9C.4** CLI flags: `--no-rotate`, `--no-blur-faces`, `--compression 0`

---

## Nhiệm vụ 10: Producer-Consumer Batch Pipeline

### Kiến trúc hiện tại (tuần tự)

```
for each image:
  read → detect → blur → write   (4-5s)
```

### Kiến trúc mới (pipeline song song)

```
Stage 1 (Reader):    read image[i]     → buffer A
Stage 2 (Detector):  detect face       → buffer B  (song song với Reader)
Stage 3 (Writer):    blur + encode     → disk      (song song với Detector)
```

Mỗi stage chạy trên thread riêng, truyền data qua bounded buffer (queue maxSize=2).

### Chi tiết

#### 10A. Pipeline infrastructure

- [ ] **10A.1** Tạo `PipelineStage` base class (abstract thread wrapper)
- [ ] **10A.2** Tạo `BoundedBuffer<T>` template (thread-safe queue with max size)
- [ ] **10A.3** Tạo `ReaderStage`: đọc ảnh từ disk → output buffer
- [ ] **10A.4** Tạo `DetectorStage`: detect faces → output buffer
- [ ] **10A.5** Tạo `WriterStage`: blur + encode → ghi disk

#### 10B. Pipeline integration

- [ ] **10B.1** Rewriter `BatchProcessor::start()` dùng pipeline thay vì worker thread pool
- [ ] **10B.2** Tách logic detect ra hàm riêng `detectFaces(cv::Mat)` (không gọi processFile)
- [ ] **10B.3** Tách logic blur ra hàm riêng `applyBlurToImage(cv::Mat, faces)`
- [ ] **10B_4** Progress tracking: mỗi stage reported progress

#### 10C. Memory management

- [ ] **10C.1** Release cv::Mat sau khi mỗi stage xong
- [ ] **10C.2** Giới hạn buffer size = 2 (không read ahead quá 2 ảnh)
- [ ] **10C.3** Memory monitoring: log RAM usage mỗi 10 ảnh

### Flow chi tiết

```
Thread Reader (reading):
  while !done:
    img = readImage(path[i])
    buffer_A.push(img)
    i++

Thread Detector (detecting):
  while !done:
    img = buffer_A.pop()
    faces = detectFaces(img)
    buffer_B.push({img, faces})
    progress++

Thread Writer (writing):
  while !done:
    {img, faces} = buffer_B.pop()
    if blurEnabled: applyBlur(img, faces)
    if compressEnabled: compress(img)
    writeImage(img, outputPath)
    done++
```

### Kết quả mong đợi

- **Batch 100 ảnh**: ~1.5-2 phút (hiện tại 7-8 phút)
- **Batch 500 ảnh**: ~8-10 phút (hiện tại 37-42 phút)
- **RAM**: < 500MB (buffer 2 ảnh ~60MB)
- **CPU**: 3 threads working concurrently, không spike 100%

---

## Thứ tự thực hiện

1. **Nhiệm vụ 8**: Detection 640px + Intel opt (15 phút)
2. **Nhiệm vụ 9**: Feature toggles (30 phút)
3. **Nhiệm vụ 10**: Producer-consumer pipeline (45 phút)

---

## Lưu ý

- **Preview**: LUÔN hiển thị dựa trên features đang bật (không tắt preview)
- **Pipeline**: phải giữ progress tracking + cancel support
- **Backward compatible**: CLI options hiện có vẫn hoạt động
- **Test**: dùng `data_test/image_source/` (2 ảnh lớn) + `data_test/example/` (2 ảnh test FP)
