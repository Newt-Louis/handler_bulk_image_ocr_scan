# AutoPhoto - System Architecture & Features Report

## 1. Overview
**AutoPhoto** is a high-performance desktop application designed for bulk processing images with privacy-focused features like automatic face blurring, watermark timestamping, and image compression. It provides both a modern GUI built with Qt Quick/QML and a fully automated headless CLI mode for server-side or scripted processing.

## 2. Core Architecture
The application follows a hybrid architecture, separating the core processing engine from the UI layer.

### 2.1 Backend Engine (C++17)
- **Face Detection**: Utilizes a deep learning approach using OpenCV's DNN module. The core model is **YuNet** (`face_detection_yunet_2023mar.onnx`), providing fast and accurate face detection.
- **Image Processing Engine (`ImageProcessor`)**:
  - Encapsulates the entire image pipeline: reading EXIF orientation, face detection, false-positive filtering, blurring, drawing timestamps, and exporting/compressing.
  - Thread-safe and designed for concurrent batch processing.
- **Batch Processing (`BatchProcessor`)**:
  - Implements a high-throughput, multi-threaded producer-consumer pipeline using `QThreadPool`.
  - Ensures responsive UI during heavy processing by delegating OpenCV operations to background worker threads.

### 2.2 Frontend GUI (Qt/QML)
- **Qt Quick Controls 2 (Material Design)**: Provides a fluid, modern, hardware-accelerated user interface.
- **Cover Flow Preview (`CoverFlowView`)**:
  - Renders a visually appealing half-circle carousel of images.
  - Generates fast scaled previews (`regenerateFast`) to maintain UI responsiveness, followed by asynchronous deep processing previews (`regenerate`).

### 2.3 Platform Abstraction
- Uses a **Platform Strategy Pattern** (`PlatformInterface`, `DesktopPlatform`, `PlatformFactory`) to abstract OS-specific integrations (e.g., File dialogs, system paths), making the codebase prepared for future Android/iOS cross-platform expansion.

## 3. Key Features

### 3.1 Privacy Masking (Face Blur)
- Multiple Blur Modes: **Gaussian Blur** and **Pixelate**.
- Advanced False Positive Filters:
  - **Box Size Filter**: Ignores unrealistically small detection boxes.
  - **Skin-color Filter**: Validates skin tones within the detected region; rejects the image entirely if the average skin ratio is below 20%.
  - **Cascade Cross-Check**: Fallback validation using Haar cascades to confirm YuNet's findings.

### 3.2 Watermark & Timestamping
- Custom or Auto Text: Display file creation/modification dates or user-defined custom text.
- Dynamic Scaling: Font sizes dynamically scale based on the image's true resolution, ensuring visibility on 4K/high-res photos.
- Drop Shadow: A distinct black drop-shadow ensures text remains readable against bright/white backgrounds (like skies).
- Flexible Positioning: Pre-defined corners (TopLeft, BottomRight, etc.) or absolute Custom X/Y coordinates (with Y mapping to the top-left corner of the text box for intuitive placement).

### 3.3 Image Compression & Formats
- Lossy compression to significantly reduce storage footprint (configurable 0-100% slider).
- Output format support for **JPG**, **PNG**, and modern **WEBP**.

### 3.4 CLI (Headless Mode)
- Passing `--mode auto` completely bypasses the GUI, triggering a high-performance batch job in the terminal.
- Extensive command-line flags (e.g., `--input`, `--output`, `--compression`, `--blur-mode`, `--detection-sensitivity`) allow complete control over the pipeline.

## 4. Build & Deployment
- Built via **CMake** for seamless integration across Windows (MSVC) and macOS/Linux.
- Supports macOS App Bundles (`.app`) with correct resource bundling using `macdeployqt` and `dylibbundler`.
- Contains packaging configurations for `.exe` (Windows) distributions.

---
*Report Generated Automatically.*
