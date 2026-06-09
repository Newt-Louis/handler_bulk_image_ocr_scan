#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build-macos}"
INSTALL_DIR="${INSTALL_DIR:-dist/AutoPhoto}"
GENERATOR="${GENERATOR:-Ninja}"
CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-}"
OpenCV_DIR="${OpenCV_DIR:-}"

case "$INSTALL_DIR" in
  /*) INSTALL_PREFIX="$INSTALL_DIR" ;;
  *) INSTALL_PREFIX="$(pwd)/$INSTALL_DIR" ;;
esac

configure_args=(-S . -B "$BUILD_DIR" -G "$GENERATOR" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX")
if [[ -n "$CMAKE_PREFIX_PATH" ]]; then
  configure_args+=("-DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH")
fi
if [[ -n "$OpenCV_DIR" ]]; then
  configure_args+=("-DOpenCV_DIR=$OpenCV_DIR")
fi

cmake "${configure_args[@]}"
cmake --build "$BUILD_DIR" --config Release
cmake --install "$BUILD_DIR" --config Release
cmake --build "$BUILD_DIR" --target package --config Release
