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
# Wipe any previous install output first. A partial/failed prior build can
# leave stale codesign metadata (_CodeSignature) mismatched against the
# actual files, which makes macdeployqt fail later with errors like
# "code object is not signed at all" / "has no resources but signature
# indicates they must be present". Always install into a clean directory.
rm -rf "$INSTALL_PREFIX"
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

# dylibbundler resolves plain "libname.dylib" deps automatically, but deps
# referenced only via @rpath (e.g. libgcc_s.1.1.dylib pulled in by
# Homebrew's gfortran/OpenCV) are NOT found unless we hand it explicit
# search paths with -s. Auto-detect common Homebrew GCC lib dirs here.
extra_search_args=()
for gcc_lib_dir in \
    /opt/homebrew/opt/gcc/lib/gcc/current \
    /opt/homebrew/opt/gcc/lib/gcc/* \
    /usr/local/opt/gcc/lib/gcc/current \
    /usr/local/opt/gcc/lib/gcc/*; do
  if [ -d "$gcc_lib_dir" ] && [ -e "$gcc_lib_dir/libgcc_s.1.1.dylib" ]; then
    extra_search_args+=(-s "$gcc_lib_dir")
  fi
done

if [ "${#extra_search_args[@]}" -eq 0 ]; then
  echo "WARNING: could not auto-locate libgcc_s.1.1.dylib under Homebrew GCC paths."
  echo "         Run: find /opt/homebrew /usr/local -name 'libgcc_s.1.1.dylib' 2>/dev/null"
  echo "         and set GCC_LIB_DIR=<that folder> to pass it explicitly."
  if [[ -n "${GCC_LIB_DIR:-}" ]]; then
    extra_search_args+=(-s "$GCC_LIB_DIR")
  fi
fi

FRAMEWORKS_DIR="$APP_DIR/Contents/Frameworks"
mkdir -p "$FRAMEWORKS_DIR"

# --- Workaround for known dylibbundler 1.0.5 bug ---------------------------
# dylibbundler cannot resolve dependencies-of-dependencies that are only
# referenced via @rpath (e.g. libgfortran needs libgcc_s / libquadmath,
# both pulled in transitively by OpenCV). It fails with:
#   "Error: An error occured while trying to set write permissions..."
# Workaround: manually copy the known GCC runtime dylibs into Frameworks/
# ourselves, fix their install-name to @rpath/<lib>, and fix cross-references
# between them. Then tell dylibbundler to leave that source directory alone
# (-i) so it doesn't try (and fail) to re-process them.
GCC_LIB_DIR_RESOLVED=""
for ((i=0; i<${#extra_search_args[@]}; i++)); do
  if [ "${extra_search_args[$i]}" = "-s" ]; then
    GCC_LIB_DIR_RESOLVED="${extra_search_args[$((i+1))]}"
    break
  fi
done

if [ -n "$GCC_LIB_DIR_RESOLVED" ]; then
  gcc_runtime_libs=(libgcc_s.1.1.dylib libgfortran.5.dylib libquadmath.0.dylib)

  echo "=== Pre-bundling GCC runtime libs from $GCC_LIB_DIR_RESOLVED ==="
  for lib in "${gcc_runtime_libs[@]}"; do
    src="$GCC_LIB_DIR_RESOLVED/$lib"
    if [ -f "$src" ]; then
      cp -f "$src" "$FRAMEWORKS_DIR/$lib"
      chmod +w "$FRAMEWORKS_DIR/$lib"
      install_name_tool -id "@rpath/$lib" "$FRAMEWORKS_DIR/$lib"
    fi
  done
  # Fix cross-references between the GCC libs themselves (e.g. libgfortran
  # depends on libgcc_s and libquadmath).
  for lib in "${gcc_runtime_libs[@]}"; do
    [ -f "$FRAMEWORKS_DIR/$lib" ] || continue
    for dep in "${gcc_runtime_libs[@]}"; do
      install_name_tool -change "@rpath/$dep" "@executable_path/../Frameworks/$dep" "$FRAMEWORKS_DIR/$lib" 2>/dev/null || true
      install_name_tool -change "@loader_path/$dep" "@executable_path/../Frameworks/$dep" "$FRAMEWORKS_DIR/$lib" 2>/dev/null || true
    done
    codesign --force --sign - "$FRAMEWORKS_DIR/$lib" 2>/dev/null || true
  done
  ignore_args=(-i "$GCC_LIB_DIR_RESOLVED")
else
  ignore_args=()
fi
# -----------------------------------------------------------------------

# NOTE: switched from -od (overwrite dir, WIPES Frameworks/ first) to
# -cd -of (create dir if missing, overwrite individual files) so the
# GCC libs we just pre-bundled above survive this step.
dylibbundler -cd -of -b \
  -x "$EXEC_FILE" \
  -d "$FRAMEWORKS_DIR/" \
  -p @executable_path/../Frameworks/ \
  "${extra_search_args[@]}" \
  "${ignore_args[@]}"

# --- Fix duplicate LC_RPATH entries -----------------------------------
# dylibbundler can call install_name_tool -add_rpath on the same file
# more than once while walking the dependency graph (common for a widely
# depended-on library like libopencv_core, which nearly every other
# OpenCV module links against). A duplicated LC_RPATH entry makes dyld on
# recent macOS refuse to search that path at all, causing errors like:
#   "Library not loaded: @executable_path/../Frameworks/libX.dylib
#    ... (duplicate LC_RPATH '@executable_path/../Frameworks/')"
# even though the file is right there. Deduplicate any repeated rpath
# entries across every Mach-O file in the bundle.
echo "=== Deduplicating rpath entries ==="
TARGET_RPATH="@executable_path/../Frameworks/"
while IFS= read -r -d '' macho_file; do
  count=$(otool -l "$macho_file" 2>/dev/null | awk -v p="$TARGET_RPATH" '$1=="path" && $2==p {c++} END{print c+0}')
  while [ "$count" -gt 1 ]; do
    install_name_tool -delete_rpath "$TARGET_RPATH" "$macho_file" 2>/dev/null || break
    count=$((count - 1))
  done
done < <(find "$APP_DIR" -type f \( -name "*.dylib" -o -perm +111 \) -print0 2>/dev/null)
# ------------------------------------------------------------------------

# --- Final re-sign, AFTER all file manipulation is done -----------------
# macdeployqt seals the bundle's code signature early. Everything we did
# afterward (copying the GCC runtime libs by hand, dylibbundler adding/
# fixing dozens of dylibs, deleting duplicate rpaths) changes file
# contents that the earlier seal already recorded hashes for. A stale/
# mismatched seal makes macOS refuse to run the app -- it gets silently
# SIGKILLed at launch (shows as "zsh: killed", no crash log) instead of
# a normal Gatekeeper prompt. Re-sign the whole bundle from scratch, deep,
# as the very last step so the seal matches the final file contents.
echo "=== Re-signing app bundle (ad-hoc, deep) ==="
codesign --force --deep --sign - "$APP_DIR"
codesign --verify --deep --strict --verbose=2 "$APP_DIR"
# ------------------------------------------------------------------------

echo "=== Creating DMG ==="
cmake --build "$BUILD_DIR" --target package --config Release

echo ""
echo "✅ Build complete!"
echo "   .app: $APP_DIR"
echo "   .dmg: $(find "$BUILD_DIR" -name "*.dmg" | head -n 1)"
echo ""
echo "To run on another Mac: copy the .app (or open the .dmg and drag to Applications)."342ewqdsafewr3qsadZX
