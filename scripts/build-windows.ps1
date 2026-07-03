# ============================================================
# AutoPhoto Windows Build Script
# Produces a portable folder + ZIP (no installer needed)
#
# Prerequisites:
#   - Visual Studio 2022 (with C++ Desktop Development workload)
#   - CMake (https://cmake.org/download)
#   - Ninja (choco install ninja)
#   - Qt 6.4+ (https://www.qt.io/download-qt-installer)
#   - OpenCV 4.x (https://opencv.org/releases)
#
# Usage (Developer PowerShell for VS 2022):
#   .\scripts\build-windows.ps1 `
#     -QtPrefix "C:\Qt\6.4.2\msvc2019_64" `
#     -OpenCvDir "C:\opencv\build"
#
# Override defaults:
#   .\scripts\build-windows.ps1 -BuildDir build-windows -SkipZip
# ============================================================

param(
    [string]$QtPrefix = $env:Qt6_DIR,
    [string]$OpenCvDir = $env:OpenCV_DIR,
    [string]$BuildDir = "build-windows",
    [string]$InstallDir = "dist\AutoPhoto",
    [string]$Generator = "Ninja",
    [switch]$SkipZip
)

$ErrorActionPreference = "Stop"

if (-not $QtPrefix) {
    throw "Qt prefix path is required. Pass -QtPrefix or set `$env:Qt6_DIR."
}

$InstallFullPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($InstallDir)

Write-Host "=== Configuring ===" -ForegroundColor Cyan
$configureArgs = @(
    "-S", ".",
    "-B", $BuildDir,
    "-G", $Generator,
    "-DCMAKE_BUILD_TYPE=Release",
    "-DCMAKE_INSTALL_PREFIX=$InstallFullPath",
    "-DCMAKE_PREFIX_PATH=$QtPrefix"
)
if ($OpenCvDir) {
    $configureArgs += "-DOpenCV_DIR=$OpenCvDir"
}

cmake @configureArgs

Write-Host "=== Building ===" -ForegroundColor Cyan
cmake --build $BuildDir --config Release

Write-Host "=== Installing (deploy Qt + OpenCV DLLs) ===" -ForegroundColor Cyan
cmake --install $BuildDir --config Release

# Ensure models folder exists in the portable output
$modelsSource = Join-Path $PSScriptRoot "..\models"
$modelsDest = Join-Path $InstallFullPath "models"
if ((Test-Path $modelsSource) -and -not (Test-Path $modelsDest)) {
    Copy-Item -Path $modelsSource -Destination $modelsDest -Recurse -Force
}

if (-not $SkipZip) {
    Write-Host "=== Creating ZIP ===" -ForegroundColor Cyan
    $zipName = "AutoPhoto-Windows-Portable.zip"
    if (Test-Path $zipName) { Remove-Item $zipName }
    Compress-Archive -Path "$InstallFullPath\*" -DestinationPath $zipName
    Write-Host ""
    Write-Host "Build complete!" -ForegroundColor Green
    Write-Host "  Portable folder: $InstallFullPath"
    Write-Host "  ZIP: $zipName"
} else {
    Write-Host ""
    Write-Host "Build complete!" -ForegroundColor Green
    Write-Host "  Portable folder: $InstallFullPath"
}

Write-Host ""
Write-Host "To run on another Windows PC: copy the portable folder (or ZIP) and run AutoPhoto.exe"
