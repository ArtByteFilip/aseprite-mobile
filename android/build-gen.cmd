@echo off
rem Compiles the "gen" tool for Windows (required to cross-compile
rem Aseprite for Android). Requires Visual Studio 2022 and CMake/Ninja
rem (from the Android SDK).
rem
rem Usage: build-gen.cmd [BUILD_DIR]

setlocal
set BUILD_DIR=%~1
if "%BUILD_DIR%"=="" set BUILD_DIR=C:\deps\build-gen-host
set SRC_DIR=%~dp0..
if "%CMAKE_DIR%"=="" set CMAKE_DIR=%LOCALAPPDATA%\Android\Sdk\cmake\3.31.6\bin

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do set VS_DIR=%%i
call "%VS_DIR%\VC\Auxiliary\Build\vcvars64.bat" >nul

mkdir "%BUILD_DIR%" 2>nul
cd /d "%BUILD_DIR%"
"%CMAKE_DIR%\cmake.exe" -G Ninja -DCMAKE_MAKE_PROGRAM="%CMAKE_DIR%\ninja.exe" ^
  -DCMAKE_BUILD_TYPE=Release -DLAF_BACKEND=none ^
  -DENABLE_UPDATER=OFF -DENABLE_NEWS=OFF -DENABLE_WEBSOCKET=OFF ^
  -DENABLE_CCACHE=OFF -DENABLE_SCRIPTING=OFF ^
  -DLAF_WITH_EXAMPLES=OFF -DLAF_WITH_TESTS=OFF "%SRC_DIR%" || exit /b 1
"%CMAKE_DIR%\ninja.exe" gen || exit /b 1
echo.
echo gen.exe compiled in %BUILD_DIR%\bin\gen.exe
