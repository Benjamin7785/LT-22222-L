@echo off
REM ============================================
REM Quick ARM GCC Toolchain Installer
REM ============================================

echo.
echo ============================================
echo ARM GCC Toolchain Quick Setup
echo ============================================
echo.

REM Check if already installed
where arm-none-eabi-gcc >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo ARM GCC is already installed!
    arm-none-eabi-gcc --version | findstr "gcc"
    echo.
    echo Ready to build!
    pause
    exit /b 0
)

echo ARM GCC not found. Let's install it...
echo.
echo.
echo Step 1: Download ARM GCC
echo ========================================
echo.
echo Opening download page in your browser...
echo Please download the Windows installer (.exe)
echo.
start https://developer.arm.com/downloads/-/gnu-rm
echo.
pause
echo.

echo.
echo Step 2: Installation Instructions
echo ========================================
echo.
echo 1. Run the downloaded .exe file
echo 2. Install to: C:\Program Files (x86)\GNU Arm Embedded Toolchain\
echo 3. IMPORTANT: Check "Add path to environment variable"
echo 4. Complete the installation
echo.
pause
echo.

echo.
echo Step 3: Install Make
echo ========================================
echo.
echo We need GNU Make to build the project.
echo.
echo Choose installation method:
echo   1. Chocolatey (easiest - will install if needed)
echo   2. Manual download
echo   3. Skip (I already have Make)
echo.
set /p MAKE_CHOICE="Enter choice (1-3): "

if "%MAKE_CHOICE%"=="1" (
    echo.
    echo Installing Chocolatey and Make...
    echo.
    echo This requires Administrator privileges.
    echo Right-click this script and "Run as Administrator" if needed.
    echo.
    pause
    
    REM Check if Chocolatey is installed
    where choco >nul 2>nul
    if %ERRORLEVEL% NEQ 0 (
        echo Installing Chocolatey...
        powershell -NoProfile -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))"
    )
    
    echo Installing Make...
    choco install make -y
    
) else if "%MAKE_CHOICE%"=="2" (
    echo.
    echo Opening MinGW download page...
    start http://www.mingw.org/
    echo.
    echo After installing MinGW:
    echo 1. Select mingw32-make
    echo 2. Add C:\MinGW\bin to PATH
    echo.
    pause
    
) else (
    echo.
    echo Skipping Make installation.
    echo.
)

echo.
echo ============================================
echo Setup Complete!
echo ============================================
echo.
echo Please RESTART Command Prompt and run:
echo   cd "Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)"
echo   build_gcc.bat
echo.
echo Or manually:
echo   make clean
echo   make all
echo.
pause



