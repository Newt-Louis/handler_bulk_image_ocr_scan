# Kế hoạch phát triển

Vì sao mặt "hơi xa" (diện tích nhỏ) bị bỏ sót
Đây gần như chắc chắn là vấn đề độ phân giải đầu vào, không phải do YuNet "không nhìn thấy được mặt nhỏ" về bản chất. Có 3 nguyên nhân thường gặp:

Resize ảnh trước khi đưa vào model: Nếu ảnh gốc 4000×3000 bị resize xuống 320×320 hay tương tự trước khi inference, một khuôn mặt 60×60px trên ảnh gốc có thể co lại còn 5×5px — dưới ngưỡng mà bất kỳ detector nào (kể cả SOTA) có thể nhận ra.
input_size của cv::FaceDetectorYN đang set cố định nhỏ thay vì theo kích thước ảnh thực tế.
score_threshold mặc định của YuNet là 0.9 — khá cao. Mặt nhỏ/mờ thường có confidence thấp hơn ngưỡng này nên bị loại trước khi tới bước filter của bạn.

Kiến trúc: YuNet là model rất nhẹ (thiết kế cho edge/real-time), dùng feature map ở độ phân giải tương đối thấp so với các detector chuyên xử lý đa tỷ lệ (multi-scale). Nó có thể detect mặt nhỏ nhưng biên độ hoạt động (operating range) hẹp hơn nhiều so với các model dùng Feature Pyramid Network (FPN) chuyên trị bài toán "vật thể to nhỏ khác nhau trong cùng 1 ảnh".
2. Vì sao false positive không liên quan đến ánh sáng
Điều này gần như xác nhận thủ phạm chính là bộ lọc skin-color (HSV). Đây là kỹ thuật từ thời Viola-Jones (2000s), rất mong manh:

Ngưỡng HSV cho "màu da" phụ thuộc tông da, camera, white balance — không phải chỉ độ sáng.
Nó dễ match nhầm gỗ, tường sơn màu be/cam, da người ở vùng không phải mặt (tay, cổ), thậm chí một số nền màu nâu/hồng.
Vì vậy dù ảnh đủ sáng hay thiếu sáng, filter này vẫn cho pass những vùng có "màu giống da" — không liên quan gì tới việc mặt có thật hay không.

Cascade cross-check (Haar) cũng là detector yếu, tự nó vốn nổi tiếng false positive cao — dùng để "xác nhận chéo" một model yếu khác không giúp tăng độ tin cậy nhiều như kỳ vọng.
3. Vậy YuNet có nên giữ không?
Có thể giữ nếu bạn tối ưu lại cách dùng, nhưng nếu muốn giải quyết triệt để bài toán "mặt xa" tôi khuyên đổi sang model có FPN đa tỷ lệ. Vì app của bạn:

Chạy trên desktop (Windows/macOS), không phải mobile/embedded → có dư tài nguyên compute.
Là xử lý ảnh tĩnh (không phải video real-time 30fps) → có thể afford model nặng hơn.
Không cần recognition, chỉ cần detection chính xác → không cần trade-off tốc độ như các app real-time.

→ Ràng buộc khiến YuNet hấp dẫn (siêu nhẹ, siêu nhanh) không thực sự áp dụng cho bạn. Bạn đang "trả giá" về recall (bỏ sót mặt xa) để đổi lấy tốc độ mà bạn không cần tận dụng.
  ::view-transition-group(*),
  ::view-transition-old(*),
  ::view-transition-new(*) {
    animation-duration: 0.25s;
    animation-timing-function: cubic-bezier(0.19, 1, 0.22, 1);
  }
VvisualizeVvisualize show_widget4. Đề xuất cụ thể theo mức độ ưu tiên
A. Fix nhanh, không cần đổi model (test trước)

Bỏ resize xuống nhỏ trước khi inference — set input_size của FaceDetectorYN::create() bằng đúng kích thước ảnh gốc (hoặc tối đa model chịu được).
Hạ score_threshold từ 0.9 xuống ~0.5–0.6, để filter phía sau lo phần loại false positive thay vì để model tự loại trước (đang loại nhầm cả true positive mờ/nhỏ).
Tiling: với ảnh lớn (>2000px cạnh dài), chia thành các tile chồng lấn ~20-30% (overlap để tránh cắt mặt ở biên), chạy detect từng tile ở độ phân giải cao, sau đó quy đổi toạ độ box về ảnh gốc và chạy NMS toàn cục để gộp/loại trùng.

Nếu sau bước này mặt xa vẫn miss nhiều → giới hạn kiến trúc của YuNet, cần đổi model.
B. Đổi model detector chính
ModelƯu điểmKhi nào chọnSCRFD (insightface)FPN đa tầng, mạnh nhất ở mặt nhỏ (WIDER FACE hard subset), có bản ONNX sẵn (2.5G/10G/34G)Khuyến nghị chính — cân bằng tốc độ/độ chính xác tốtRetinaFace (mobilenet0.25 hoặc resnet50)Rất chính xác, cũng FPN, có landmark 5 điểmNếu cần độ chính xác cao nhất, chấp nhận chậm hơnYOLOv8-faceMulti-scale tự nhiên qua PANet, dễ tích hợp nếu quen hệ YOLOLựa chọn thay thế tốt
Vì bạn đã có sẵn pipeline ONNX Runtime/OpenCV DNN cho YuNet, việc thay model chỉ là đổi file .onnx + viết lại hàm decode output (khác format một chút so với YuNet) — không phải viết lại kiến trúc app. Đây là lý do nên cân nhắc nghiêm túc, chi phí kỹ thuật không lớn.
C. Thiết kế lại bộ lọc false positive

Bỏ hẳn skin-color HSV filter — đây là nghi phạm chính gây false positive không liên quan ánh sáng, vì nó match theo tông màu chứ không theo cấu trúc khuôn mặt.
Thay bằng kiểm tra hình học 5 landmark mà YuNet/SCRFD/RetinaFace đều trả về sẵn: 2 mắt, mũi, 2 khoé miệng. Kiểm tra:

2 mắt roughly cùng độ cao (lệch góc trong ngưỡng cho phép)
mũi nằm giữa 2 mắt theo chiều ngang
khoảng cách mắt-mắt, mắt-miệng theo tỷ lệ hợp lý so với kích thước box

Đây là filter rẻ (không cần thêm inference), robust hơn nhiều so với màu da vì dựa vào cấu trúc chứ không phải màu sắc.
Giữ size filter (vẫn hợp lý để loại box quá nhỏ/quá to bất thường).
Cascade cross-check: có thể bỏ nếu model mới đủ tốt — Haar cascade tự nó cũng gây noise, "2 detector yếu chéo nhau" không chắc tốt hơn "1 detector mạnh + 1 filter hình học".
Aggregate skin ratio reject-all: nên bỏ luôn theo skin-color filter vì cùng logic yếu.

5. Lộ trình triển khai đề xuất

Test nhanh input_size + threshold + tiling trên YuNet hiện tại (1-2 ngày) → đo lại recall trên bộ ảnh có mặt xa.
Nếu chưa đạt, tích hợp SCRFD-10G ONNX song song để so sánh A/B trên cùng bộ test ảnh (mặt gần, mặt xa, ảnh thiếu sáng, ảnh nhiều mặt).
Viết lại filter: bỏ skin-color, thêm landmark-geometry check.
Tune lại NMS IoU threshold khi merge tile để tránh double-box ở biên tile.
Benchmark tốc độ trên máy target (Windows/macOS) để đảm bảo UX chấp nhận được (vì đây là xử lý ảnh tĩnh nên có độ trễ vài trăm ms/ảnh thường vẫn ổn).