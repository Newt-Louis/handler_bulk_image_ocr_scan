#!/usr/bin/env bash
set -euo pipefail

# ============================================================
# AutoPhoto macOS Build Script
# Produces a self-contained .app bundle + .dmg
#
# Prerequisites (install once):
#   brew install cmake ninja qt@6 opencv dylibbundler
#
# Usage:
#   bash scripts/build-macos.sh
#
# Override defaults with env vars:
#   BUILD_DIR=build-macos INSTALL_DIR=dist/AutoPhoto bash scripts/build-macos.sh
# ============================================================

BUILD_DIR="${BUILD_DIR:-build-macos}"
INSTALL_DIR="${INSTALL_DIR:-dist/AutoPhoto}"
GENERATOR="${GENERATOR:-Ninja}"
CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-}"
OpenCV_DIR="${OpenCV_DIR:-}"

case "$INSTALL_DIR" in
  /*) INSTALL_PREFIX="$INSTALL_DIR" ;;
  *)  INSTALL_PREFIX="$(pwd)/$INSTALL_DIR" ;;
esac

echo "=== Configuring ==="
configure_args=(
  -S . -B "$BUILD_DIR"
  -G "$GENERATOR"
  -DCMAKE_BUILD_TYPE=Release
  "-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX"
)
[[ -n "$CMAKE_PREFIX_PATH" ]] && configure_args+=("-DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH")
[[ -n "$OpenCV_DIR" ]] && configure_args+=("-DOpenCV_DIR=$OpenCV_DIR")

cmake "${configure_args[@]}"

echo "=== Building ==="
cmake --build "$BUILD_DIR" --config Release

echo "=== Installing ==="
cmake --install "$BUILD_DIR" --config Release

# Locate the installed .app bundle
APP_DIR=$(find "$INSTALL_PREFIX" -maxdepth 1 -name "*.app" -type d | head -n 1)

if [ -z "$APP_DIR" ]; then
  echo "ERROR: No .app bundle found in $INSTALL_PREFIX"
  ls -la "$INSTALL_PREFIX"
  exit 1
fi

echo "=== Deploying Qt frameworks (macdeployqt) ==="
macdeployqt "$APP_DIR" -verbose=1

echo "=== Deploying OpenCV dylibs (dylibbundler) ==="
EXEC_FILE=$(find "$APP_DIR/Contents/MacOS" -type f -perm +111 | head -n 1)
echo "Executable: $EXEC_FILE"
mkdir -p "$APP_DIR/Contents/Frameworks/"
dylibbundler -od -b \
  -x "$EXEC_FILE" \
  -d "$APP_DIR/Contents/Frameworks/" \
  -p @executable_path/../Frameworks/

echo "=== Creating DMG ==="
cmake --build "$BUILD_DIR" --target package --config Release

echo ""
echo "✅ Build complete!"
echo "   .app: $APP_DIR"
echo "   .dmg: $(find "$BUILD_DIR" -name "*.dmg" | head -n 1)"
echo ""
echo "To run on another Mac: copy the .app (or open the .dmg and drag to Applications)."
