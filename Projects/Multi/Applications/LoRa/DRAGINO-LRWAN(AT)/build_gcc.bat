@echo off
REM ============================================
REM GCC Build Script for LoRa P2P Enhanced
REM ============================================

echo.
echo ============================================
echo Building LoRa P2P Enhanced with GCC
echo ============================================
echo.

REM Check if ARM GCC is in PATH
where arm-none-eabi-gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: ARM GCC not found in PATH!
    echo.
    echo Please install ARM GCC from:
    echo https://developer.arm.com/downloads/-/gnu-rm
    echo.
    echo Or add it to PATH:
    echo set PATH=C:\Program Files (x86)\GNU Arm Embedded Toolchain\12.2 2022.12\bin;%%PATH%%
    echo.
    pause
    exit /b 1
)

echo ARM GCC found!
arm-none-eabi-gcc --version | findstr "gcc"
echo.

REM Clean previous build
if exist build (
    echo Cleaning previous build...
    rmdir /S /Q build
    echo.
)

REM Build
echo Starting build...
echo.
make -j8 all

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ============================================
    echo BUILD SUCCESSFUL!
    echo ============================================
    echo.
    echo Output files:
    echo   ELF: build\LoRa_P2P_Enhanced.elf
    echo   HEX: build\LoRa_P2P_Enhanced.hex
    echo   BIN: build\LoRa_P2P_Enhanced.bin
    echo   MAP: build\LoRa_P2P_Enhanced.map
    echo.
    echo To flash: build_gcc.bat flash
    echo.
) else (
    echo.
    echo ============================================
    echo BUILD FAILED!
    echo ============================================
    echo Check errors above.
    echo.
)

pause


