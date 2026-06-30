@echo off
cd /d "%~dp0"

REM Clear cached ImGui source (switched to docking branch)
if exist "build\_deps\imgui-src" rmdir /s /q "build\_deps\imgui-src"
if exist "build\_deps\imgui-build" rmdir /s /q "build\_deps\imgui-build"
if exist "build\_deps\imgui-subbuild" rmdir /s /q "build\_deps\imgui-subbuild"

if not exist "build" mkdir build
cd build

cmake .. -G "Visual Studio 18 2026" -A x64
if %ERRORLEVEL% neq 0 (
    cmake .. -G "Visual Studio 17 2022" -A x64
)
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configure failed.
    pause
    exit /b 1
)

cmake --build . --config Release --parallel
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

copy Release\BadKB.exe ..\BadKB.exe >nul 2>&1
echo.
echo [OK] BadKB.exe built successfully.
pause
