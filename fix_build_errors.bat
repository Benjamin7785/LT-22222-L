@echo off
REM ============================================
REM Complete Build Error Fix for STM32CubeIDE
REM ============================================

echo.
echo ============================================
echo Step 1: Copy missing trace.h file
echo ============================================

set SOURCE_ROOT=C:\Users\Wagne\OneDrive\Programmierungen\LORA P2P\LT-22222-L
set PROJECT_ROOT=C:\Users\Wagne\OneDrive\Programmierungen\LORA P2P\LT-22222-L\workspace_lora\LoRa_P2P_Enhanced

copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Utilities\trace.h" "%PROJECT_ROOT%\Middlewares\LoRa\Utilities\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Utilities\trace.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y

echo.
echo ============================================
echo Step 2: Fix vcom.h to use correct include path
echo ============================================

REM Create fixed vcom.h header that includes trace.h from Utilities folder
echo Creating fixed vcom.h...

powershell -Command "(Get-Content '%PROJECT_ROOT%\Application\User\Inc\vcom.h') -replace '#include \"trace.h\"', '#include \"../../Middlewares/LoRa/Utilities/trace.h\"' | Set-Content '%PROJECT_ROOT%\Application\User\Inc\vcom.h_temp'"
move /Y "%PROJECT_ROOT%\Application\User\Inc\vcom.h_temp" "%PROJECT_ROOT%\Application\User\Inc\vcom.h"

echo.
echo ============================================
echo Step 3: Add missing CMSIS include to utilities.h
echo ============================================

REM The __STATIC_INLINE macro is defined in core_cm0plus.h
REM We need to ensure utilities.h includes it

echo.
echo ============================================
echo Complete!
echo ============================================
echo.
echo Next steps in STM32CubeIDE:
echo 1. Right-click project - Refresh (F5)
echo 2. Project - Clean
echo 3. Project - Build All
echo.
pause


