@echo off
echo ========================================
echo Solution Generator
echo ========================================
echo.

REM 1. Prepare build directory
if not exist "generated-vs" (
    mkdir generated-vs
)

cd generated-vs

REM 2. Run CMake (Visual Studio 2022, x64)
echo [INFO] Generating Visual Studio solution...
cmake .. -G "Visual Studio 17 2022" -A x64

if errorlevel 1 (
    echo [ERROR] CMake configuration failed!
    echo.
    pause
    exit /b 1
)

echo [SUCCESS] Solution generated successfully!
echo.
cd ..

pause
