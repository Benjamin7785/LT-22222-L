@echo off
REM ============================================
REM GCC Build using STM32CubeIDE's Make
REM ============================================

echo.
echo ============================================
echo Building with GCC (using CubeIDE Make)
echo ============================================
echo.

REM Find STM32CubeIDE installation
set CUBEIDE_PATH=C:\ST\STM32CubeIDE_1.13.0
if not exist "%CUBEIDE_PATH%" set CUBEIDE_PATH=C:\ST\STM32CubeIDE_1.12.0
if not exist "%CUBEIDE_PATH%" set CUBEIDE_PATH=C:\ST\STM32CubeIDE_1.11.0
if not exist "%CUBEIDE_PATH%" set CUBEIDE_PATH=C:\ST\STM32CubeIDE_1.10.0

if not exist "%CUBEIDE_PATH%" (
    echo ERROR: STM32CubeIDE not found!
    echo Please update CUBEIDE_PATH in this script.
    pause
    exit /b 1
)

REM Find Make in CubeIDE
for /d %%i in ("%CUBEIDE_PATH%\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.make.win32_*") do (
    set MAKE_PATH=%%i\tools\bin
)

if not exist "%MAKE_PATH%\make.exe" (
    echo ERROR: Make not found in STM32CubeIDE!
    echo Expected: %MAKE_PATH%
    pause
    exit /b 1
)

echo Found CubeIDE: %CUBEIDE_PATH%
echo Found Make: %MAKE_PATH%
echo.

REM Check ARM GCC
where arm-none-eabi-gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: ARM GCC not found in PATH!
    echo Please install from: https://developer.arm.com/downloads/-/gnu-rm
    pause
    exit /b 1
)

echo ARM GCC found!
arm-none-eabi-gcc --version | findstr "gcc"
echo.

REM Add Make to PATH for this session
set PATH=%MAKE_PATH%;%PATH%

REM Clean previous build
if exist build (
    echo Cleaning previous build...
    rmdir /S /Q build
    echo.
)

REM Build
echo Starting build...
echo.
"%MAKE_PATH%\make.exe" -j8 all

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ============================================
    echo BUILD SUCCESSFUL!
    echo ============================================
    echo.
    echo Output files:
    echo   HEX: build\LoRa_P2P_Enhanced.hex
    echo   BIN: build\LoRa_P2P_Enhanced.bin
    echo   ELF: build\LoRa_P2P_Enhanced.elf
    echo.
) else (
    echo.
    echo ============================================
    echo BUILD FAILED!
    echo ============================================
    echo.
)

pause



