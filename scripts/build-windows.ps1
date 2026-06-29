# On Laptop Lennovo, command for "Developer Command Prompt for VS 2026" is:
# cmake -B build -DCMAKE_PREFIX_PATH="D:\Qt\6.9.3\msvc2022_64" -DOpenCV_DIR="D:\opencv\build\x64\vc16\lib"

param(
    [string]$QtPrefix = $env:Qt6_DIR,
    [string]$OpenCvDir = $env:OpenCV_DIR,
    [string]$BuildDir = "build-windows",
    [string]$InstallDir = "dist\AutoPhoto",
    [string]$Generator = "Ninja",
    [switch]$SkipInstaller
)

$ErrorActionPreference = "Stop"

$InstallFullPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($InstallDir)
$configureArgs = @(
    "-S", ".",
    "-B", $BuildDir,
    "-G", $Generator,
    "-DCMAKE_BUILD_TYPE=Release",
    "-DCMAKE_INSTALL_PREFIX=$InstallFullPath"
)
if ($QtPrefix) {
    $configureArgs += "-DCMAKE_PREFIX_PATH=$QtPrefix"
}
if ($OpenCvDir) {
    $configureArgs += "-DOpenCV_DIR=$OpenCvDir"
}

cmake @configureArgs
cmake --build $BuildDir --config Release
cmake --install $BuildDir --config Release

if (-not $SkipInstaller) {
    $iss = Join-Path $BuildDir "packaging\windows\AutoPhoto.iss"
    $iscc = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"
    if (-not (Test-Path $iscc)) {
        throw "Inno Setup 6 was not found. Install it or rerun with -SkipInstaller. Expected: $iscc"
    }
    & $iscc $iss
}
