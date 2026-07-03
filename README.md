# AutoPhoto

AutoPhoto is a C++17 Qt Quick desktop app for batch image processing.
It provides a Cover Flow image browser, real-time processed preview,
multi-worker batch export, face censoring with OpenCV/YuNet, and a headless
CLI mode for automation.

## Stack

- C++17
- Qt 6 Quick/QML with Qt Quick Controls 2 Material style
- CMake
- Optional OpenCV 4 integration hook

## Build From Source

Desktop packages must be built on the target OS.
Pull this source code onto each OS and run the corresponding build script.

---

### 🍎 macOS

**1. Install prerequisites (one-time):**

```bash
brew install cmake ninja qt@6 opencv dylibbundler
```

If `qt@6` is keg-only, add it to your PATH:

```bash
echo 'export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

**2. Build:**

```bash
bash scripts/build-macos.sh
```

**3. Output:**

- `.app` bundle in `dist/AutoPhoto/AutoPhoto.app` — self-contained, copy to any Mac
- `.dmg` in `build-macos/` — disk image for distribution

**Override defaults:**

```bash
BUILD_DIR=my-build INSTALL_DIR=my-dist bash scripts/build-macos.sh
```

---

### 🪟 Windows

**1. Install prerequisites (one-time):**

- [Visual Studio 2022](https://visualstudio.microsoft.com/) — select "Desktop development with C++"
- [CMake](https://cmake.org/download/) (3.24+)
- [Ninja](https://ninja-build.org/) (`choco install ninja`)
- [Qt 6.4+](https://www.qt.io/download-qt-installer) — install MSVC 2019 64-bit component
- [OpenCV 4.x](https://opencv.org/releases/) — extract to e.g. `C:\opencv`

**2. Build (run in "Developer PowerShell for VS 2022"):**

```powershell
.\scripts\build-windows.ps1 `
  -QtPrefix "C:\Qt\6.11.1\msvc2022_64" `
  -OpenCvDir "C:\opencv\build\x64\vc16"
```

**3. Output:**

- Portable folder in `dist\AutoPhoto\` — contains `AutoPhoto.exe` + all DLLs + models
- `AutoPhoto-Windows-Portable.zip` — zip archive for distribution

**Override defaults:**

```powershell
.\scripts\build-windows.ps1 -BuildDir my-build -InstallDir "C:\my-dist" -SkipZip
```

---

### 🐧 Linux (Ubuntu/Debian)

**1. Install prerequisites (one-time):**

```bash
sudo apt update
sudo apt install cmake ninja-build qt6-base-dev qt6-declarative-dev \
  qt6-shadertools-dev libopencv-dev \
  qml6-module-qtqml qml6-module-qtquick qml6-module-qtquick-controls \
  qml6-module-qtquick-dialogs qml6-module-qtquick-layouts \
  qml6-module-qtquick-window
```

**2. Build:**

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**3. Run:**

```bash
./build/autophoto
```

---

## Portable Usage

Each build script produces a **self-contained portable output**:

| OS | Output | How to use on another machine |
|----|--------|-------------------------------|
| macOS | `AutoPhoto.app` | Copy the `.app` to any Mac, double-click to run |
| Windows | `dist\AutoPhoto\` folder | Copy the folder, run `AutoPhoto.exe` |
| Linux | `build/autophoto` | Requires Qt/OpenCV installed on target machine |

## Headless Mode

The executable can process a folder without opening the GUI:

```bash
./build/autophoto \
  --mode auto \
  --input /path/to/images \
  --output /path/to/output \
  --rename-pattern autophoto \
  --blur-mode gaussian \
  --strength 100
```

Use `--no-blur-faces` to export without face censoring.

## Current Behavior

- GUI mode opens a Qt Quick application.
- Select a source folder to populate the Cover Flow.
- The selected image gets a processed preview generated on a worker thread.
- The right setup panel includes output naming and face blur.
- Batch processing uses multiple workers and OpenCV when available: it reads
  each image with resolved orientation, detects
  frontal faces with YuNet, applies Gaussian or pixelate censoring, and writes
  renamed output files.
- Images avoid blind EXIF rotation when metadata would turn an already-correct
  landscape source image sideways during preview or export.
- The YuNet model file must exist at `models/face_detection_yunet_2023mar.onnx`
  beside the project or deployed executable. CMake copies this file next to the
  built executable after each build.
- The GUI includes progress, pause, resume, stop, worker count, exported count,
  and failed count.
