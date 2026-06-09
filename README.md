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

Install Qt 6 and CMake first. OpenCV is optional for the current scaffold.
On Ubuntu/Debian, the GUI runtime also needs the QML import packages:

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

```bash
cmake -S . -B build -DAUTOPHOTO_ENABLE_OPENCV=OFF
cmake --build build
./build/autophoto
```

When OpenCV is installed and discoverable by CMake:

```bash
cmake -S . -B build -DAUTOPHOTO_ENABLE_OPENCV=ON
cmake --build build
```

On Linux the executable is `build/autophoto`. On Windows/macOS release builds
use the product name: `AutoPhoto.exe` and `AutoPhoto.app`.

## Desktop Packages

Desktop packages must be built on the target OS, or with a matching cross
toolchain. A Linux machine cannot directly produce a normal signed Windows
`.exe` installer or macOS `.app` bundle without those platform tools.

### Windows `.exe` and Installer

Install Qt 6, CMake, Ninja, OpenCV, and Inno Setup 6. Then run in PowerShell:

```powershell
.\scripts\build-windows.ps1 `
  -QtPrefix C:\Qt\6.7.0\msvc2019_64 `
  -OpenCvDir C:\opencv\build
```

The script configures a Release build, installs a deployable app into
`dist\AutoPhoto`, runs Qt deployment, and builds an Inno Setup installer from
`packaging/windows/AutoPhoto.iss`. The installer registers `AutoPhoto.exe` in
Windows App Paths so automation can call it from the shell.

Use `-SkipInstaller` to stop after producing the deployable app folder.

### macOS `.app` and `.dmg`

Install Qt 6, CMake, Ninja, and OpenCV. Then run on macOS:

```bash
bash scripts/build-macos.sh
```

The script builds Release, installs `AutoPhoto.app` into `dist/AutoPhoto`, runs
Qt deployment, and asks CPack to create a `.dmg`.

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
