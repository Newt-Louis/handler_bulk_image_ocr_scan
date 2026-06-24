# PLAN.md - Giai đoạn mới: UI polish & Hiệu năng

## Bối cảnh

Các tính năng chính đã hoàn thành (detection sensitivity, false positive filters, compression, platform scaffold). 3 bug đã fix (false positive, slow preview, toolPanel layout).

Tuy nhiên còn 2 vấn đề nghiêm trọng cần giải quyết trước khi xuất bản:
1. ScrollBar menu phải vẫn đè lên content → cần fix layout
2. CPU/RAM usage cực cao khi chuyển ảnh → ứng dụng lag, giật, có thể gây crash

---

## Nhiệm vụ 6: Fix ScrollBar Menu Phải + UI Modernization

### Vấn đề

ScrollBar của ToolPanel vẫn đè lên cạnh bên trái của menu, nhìn xấu và không chuyên nghiệp. Cần tách scrollbar ra ngoài, layout lại cho sạch sẽ.

### Chi tiết thực hiện

- [ ] **6.1** Phân tích layout hiện tại: Flickable + Column + ScrollBar overlay
- [ ] **6.2** Tách ScrollBar ra khỏi Flickable, đặt bên phải panel (không overlap content)
- [ ] **6.3** Đảm bảo content Column có width chính xác = panel width - scrollbar width - margins
- [ ] **6.4** Kiểm tra GroupBox title và content không bị cut
- [ ] **6.5** Test trên various panel widths (344px hiện tại)
- [ ] **6.6** Review overall Material Design styling: bo tròn, spacing, shadow, color consistency
- [ ] **6.7** Test QML lint không có warning mới

### Kết quả mong đợi

- ScrollBar nằm bên phải panel, không đè lên content
- Panel scroll mượt, content hiển thị đầy đủ
- Giao diện sạch sẽ, hiện đại hơn

---

## Nhiệm vụ 7: Tối ưu Hiệu năng CPU/RAM

### Vấn đề

Mỗi lần chuyển ảnh trong Cover Flow:
- YuNet model load lại (nếu chưa cached) → CPU spike
- Face detection chạy full pipeline trên ảnh gốc (có thể 4000x3000px) → RAM consumption cao
- Preview generation mất 1-3s →用户体验很差
- CPU usage nhảy lên 80-100%, RAM có thể lên vài GB
- Trên laptop core i5 Gen 12 + 24GB RAM vẫn khó khăn

### Root cause analysis

1. **YuNet model reload**: `detectWithYuNet()` dùng `thread_local cv::dnn::Net` nhưng mỗi thread có thể load lại model
2. **Full-resolution detection**: Ảnh 4000x3000 chạy face detection trên pixel gốc → rất chậm
3. **No image caching**: Mỗi preview request đọc lại ảnh từ disk
4. **No face detection caching**: Cùng một ảnh, chuyển đi chuyển lại → detect lại mỗi lần
5. **QML Image reload**: CoverFlow Image component reload khi source URL thay đổi
6. **Memory leak potential**: Worker threads có thể không release OpenCV Mat đúng cách

### Chi tiết thực hiện

#### 7A. Ảnh thu nhỏ cho detection

- [ ] **7A.1** Trước khi chạy YuNet, resize ảnh xuống max 1200px (longest side)
- [ ] **7A.2** Face detection chạy trên ảnh resized → nhanh hơn 5-10x
- [ ] **7A.3** Sau detection, map tọa độ boxes về ảnh gốc (scale factor)
- [ ] **7A.4** Blur vẫn áp dụng trên ảnh gốc (không resize)
- [ ] **7A.5** Verify: detection accuracy không giảm đáng kể

#### 7B. Face detection caching

- [ ] **7B.1** Cache kết quả face detection theo file path + detectionSensitivity
- [ ] **7B.2** Nếu ảnh đã detect với cùng sensitivity → reuse cached boxes
- [ ] **7B.3** Cache storage: QHash<QString, QVector<cv::Rect>> trong ImageProcessor hoặc singleton
- [ ] **7B.4** Cache limit: giữ tối đa 50 kết quả gần nhất (LRU)

#### 7C. Ảnh caching

- [ ] **7C.1** Cache ảnh đã đọc (QImage/cv::Mat) theo file path
- [ ] **7C.2** Nếu ảnh unchanged (file size + mtime) → reuse cached image
- [ ] **7C.3** Cache limit: giữ tối đa 20 ảnh (LRU), mỗi ảnh tối đa 50MB

#### 7D. YuNet model singleton

- [ ] **7D.1** Đảm bảo YuNet model chỉ load 1 lần duy nhất (không thread_local)
- [ ] **7D.2** Dùng static singleton pattern cho cv::dnn::Net
- [ ] **7D.3** Giải phóng model khi application quit

#### 7E. PreviewController tối ưu

- [ ] **7F.1** `regenerateFast()`: skip face detection, chỉ downscale + save (đã có)
- [ ] **7F.2** `regenerate()`: chỉ chạy full pipeline khi blurFaces == true
- [ ] **7F.3** Nếu blurFaces == false → skip face detection entirely, chỉ downscale
- [ ] **7F.4** Debounce: nếu request mới đến trong 200ms → cancel request cũ

#### 7F. Memory management

- [ ] **7G.1** Release cv::Mat sau khi process xong (scope-based)
- [ ] **7G.2** Giới hạn worker threads: max 2 thay vì hardware_concurrency
- [ ] **7G.3** QML Image cache limit:设置 imageCacheSize
- [ ] **7G.4** Monitor memory usage trong batch processing

#### 7G. Batch processing tối ưu

- [ ] **7H.1** Giảm worker count xuống max 2 (thay vì 4-8)
- [ ] **7H.2** Mỗi worker process 1 ảnh tại 1 thời điểm (không read ahead)
- [ ] **7H.3** Giải phóng memory sau mỗi ảnh (không giữ reference)
- [ ] **7H.4** Thêm delay 50ms giữa các ảnh để system喘息

### Kết quả mong đợi

- CPU usage khi chuyển ảnh: < 30% (thay vì 80-100%)
- RAM usage: < 500MB (thay vì vài GB)
- Preview load time: < 500ms (thay vì 1-3s)
- Batch processing: ổn định, không crash

### Test cases

- [ ] Test chuyển ảnh liên tục 20 lần trong 10s → CPU < 50%, RAM < 800MB
- [ ] Test batch 50 ảnh lớn (4000x3000) → hoàn thành trong 2 phút, RAM < 1GB
- [ ] Test trên laptop yếu (4GB RAM) → không crash

---

## Thứ tự thực hiện ưu tiên

### Phase 1: Nhiệm vụ 6 (UI fix)
6.1 → 6.2 → 6.3 → 6.4 → 6.5 → 6.6 → 6.7

### Phase 2: Nhiệm vụ 7 (Performance)
7A → 7B → 7C → 7D → 7E → 7F → 7G → 7H

Ưu tiên: 7A (resize detection) > 7D (model singleton) > 7B (face cache) > 7C (image cache) > còn lại

---

## Lưu ý khi implement

- **Không thay đổi detection accuracy**: resize ảnh cho detection nhưng blur vẫn trên ảnh gốc
- **Cache invalidation**: nếu file ảnh thay đổi → invalidate cache
- **Backward compatible**: tất cả CLI options hiện có vẫn hoạt động
- **Test trên ảnh thật**: dùng `data_test/example/` và `data_test/image_source/`
- **Không optimize premature**: implement theo thứ tự ưu tiên, test mỗi bước
