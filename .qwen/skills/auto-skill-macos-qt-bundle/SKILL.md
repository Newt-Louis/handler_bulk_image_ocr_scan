---
name: macos-qt-bundle
description: Correct macOS .app bundle structure, resource paths, and deployment order for Qt/C++ apps
source: auto-skill
extracted_at: '2026-07-03T06:04:06.205Z'
---

# macOS Qt .app Bundle Packaging

## .app Bundle Directory Structure

```
MyApp.app/
├── Contents/
│   ├── MacOS/          ← executable lives here
│   │   └── myapp
│   ├── Resources/      ← app resources (models, icons, configs)
│   │   └── models/
│   ├── Frameworks/     ← bundled dylibs (Qt frameworks, OpenCV, etc.)
│   └── Info.plist
```

## applicationDirPath() on macOS

`QCoreApplication::applicationDirPath()` returns `MyApp.app/Contents/MacOS/` (the directory containing the executable, NOT the bundle root).

To find resources in `Contents/Resources/`, use relative path `../Resources/` from applicationDirPath:

```cpp
// Correct — finds Contents/Resources/models/
QDir(QCoreApplication::applicationDirPath()).filePath("../Resources/models")

// Wrong — finds Contents/MacOS/models/ (not a standard location)
QDir(QCoreApplication::applicationDirPath()).filePath("models")
```

## CMake Install Paths for macOS Bundle

When installing resources into a macOS .app bundle, use:

```cmake
# Install models to the standard Resources location
install(FILES "${MODEL_FILE}" DESTINATION "MyApp.app/Contents/Resources/models")
```

NOT `MyApp.app/Contents/MacOS/models` — that puts resources next to the binary, which is non-standard and breaks when macdeployqt restructures the bundle.

## Deployment Order (Critical)

Deploy steps MUST run AFTER the build produces the .app bundle. Common mistake: placing deploy steps before configure/build in CI scripts.

Correct order:
1. `cmake -S . -B build` (configure)
2. `cmake --build build` (build → produces .app)
3. `macdeployqt build/MyApp.app` (bundle Qt frameworks)
4. `dylibbundler` (bundle third-party dylibs like OpenCV)
5. `cpack -G DragNDrop` (create .dmg)

## dylibbundler for OpenCV

```bash
EXEC_FILE=$(find "$APP_DIR/Contents/MacOS" -type f -perm +111 | head -n 1)
mkdir -p "$APP_DIR/Contents/Frameworks/"
dylibbundler -od -b \
  -x "$EXEC_FILE" \
  -d "$APP_DIR/Contents/Frameworks/" \
  -p @executable_path/../Frameworks/
```

## Model Search Pattern (Recommended)

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
