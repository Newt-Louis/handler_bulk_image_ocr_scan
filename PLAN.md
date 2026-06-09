# PLAN.md - Viec can xu ly tiep theo

## Van de hien tai: Face detector dang qua nhay

Hieu ung che mat hien tai da dat dung huong mong muon:
- Blur/mask da manh hon ban cu.
- Vung che co dang oval/privacy mask gan voi mau `data_test/example/half_face.jpg`.
- Default strength da tang len `100`, nen mac dinh che mat rat manh.

Tuy nhien detector hien tai dang bi qua nhay:
- Nguong YuNet dang la `0.35`, thap hon ban cu `0.75`.
- Viec ha nguong giup bat duoc mat nho, mat nghieng, mat chi lo 1/2 hoac 1/3.
- Nhung nguong nay cung lam phat sinh false positive: anh khong co khuon mat van bi detect va bi che nham.
- Mot so vat the may moc, vat dung, vung nen, hoac hinh dang giong mat co the bi che sai.

## Muc tieu cho lan lam tiep theo

Can dieu chinh lai detector de can bang hai yeu cau:
1. Bat duoc khuon mat partial, ke ca chi thay 1/2 hoac 1/3 khuon mat.
2. Khong che nham cac anh/vung khong co khuon mat.

## Huong xu ly nen thu

- Khong giam do manh cua blur/mask, vi nguoi dung da xac nhan blur manh la dung.
- Giu privacy mask oval hien tai, chi tinh chinh detection.
- Can tach confidence cua tung nguon detector:
  - YuNet box diem cao thi chap nhan.
  - YuNet box diem thap can qua them bo loc.
  - Haar cascade/profile chi nen dung nhu fallback co dieu kien, khong nen tin tuyet doi.
- Thu cac nguong YuNet trung gian, vi `0.35` dang qua nhay:
  - `0.40`
  - `0.45`
  - `0.50`
- Them bo loc false positive:
  - Loc box qua nho hoac qua lon so voi kich thuoc anh.
  - Loc ty le khung hinh bat thuong.
  - Loc theo skin-color ratio trong ROI neu hop ly.
  - Neu cascade box khong trung/gan YuNet box va khong co mau da ro rang thi bo qua.
- Test bat buoc tren:
  - `data_test/example/half_face.jpg` de kiem tra style che va mat partial.
  - Anh co nguoi that trong `data_test/image_source/20260416_091122.jpg`.
  - Mot so anh khong co nguoi/khuon mat de do false positive.

## Luu y khi test

- Khong xuat them file co prefix `orientfix*`.
- Neu can test export, xuat ra `/tmp/...` thay vi ghi vao `data_test/output`, tru khi nguoi dung yeu cau.
- Sau moi lan doi detector, can build:

```bash
cmake --build build --target autophoto
```

- Neu co doi QML hoac preview cache, chay them:

```bash
cmake --build build --target autophoto_qmllint
```