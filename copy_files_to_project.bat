@echo off
REM Script to copy all necessary files to STM32CubeIDE project
REM Run this from project root directory

echo ====================================
echo Copying LoRa P2P Enhanced Files
echo ====================================

set SOURCE_ROOT=C:\Users\Wagne\OneDrive\Programmierungen\LORA P2P\LT-22222-L
set PROJECT_ROOT=C:\Users\Wagne\OneDrive\Programmierungen\LORA P2P\LT-22222-L\workspace_lora\LoRa_P2P_Enhanced

echo.
echo Creating directory structure...
mkdir "%PROJECT_ROOT%\Application\User\Src" 2>nul
mkdir "%PROJECT_ROOT%\Application\User\Inc" 2>nul
mkdir "%PROJECT_ROOT%\Middlewares\LoRa\Phy" 2>nul
mkdir "%PROJECT_ROOT%\Middlewares\LoRa\Utilities" 2>nul
mkdir "%PROJECT_ROOT%\Middlewares\LoRa\Crypto" 2>nul

echo.
echo Copying Application Source Files...
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\main.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\at.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\command.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\lora.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\test_rf.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\vcom.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\debug.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\hw_gpio.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\hw_rtc.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\hw_spi.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\tiny_sscanf.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\tiny_vsnprintf.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\stm32l0xx_hal_msp.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\stm32l0xx_hw.c" "%PROJECT_ROOT%\Application\User\Src\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\src\stm32l0xx_it.c" "%PROJECT_ROOT%\Application\User\Src\" /Y

echo.
echo Copying Application Header Files...
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\at.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\command.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\lora.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\test_rf.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\vcom.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\debug.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\hw.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\hw_conf.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\hw_gpio.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\hw_msp.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\hw_rtc.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\hw_spi.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\utilities_conf.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\version.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\Commissioning.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\stm32l0xx_hal_conf.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\stm32l0xx_it.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\stm32l0xx_hw_conf.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\tiny_sscanf.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y
copy "%SOURCE_ROOT%\Projects\Multi\Applications\LoRa\DRAGINO-LRWAN(AT)\inc\tiny_vsnprintf.h" "%PROJECT_ROOT%\Application\User\Inc\" /Y

echo.
echo Copying LoRa Middleware Files...
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Phy\radio.c" "%PROJECT_ROOT%\Middlewares\LoRa\Phy\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Phy\radio.h" "%PROJECT_ROOT%\Middlewares\LoRa\Phy\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Phy\sx1276.c" "%PROJECT_ROOT%\Middlewares\LoRa\Phy\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Phy\sx1276.h" "%PROJECT_ROOT%\Middlewares\LoRa\Phy\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Phy\sx1276Regs-Fsk.h" "%PROJECT_ROOT%\Middlewares\LoRa\Phy\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Phy\sx1276Regs-LoRa.h" "%PROJECT_ROOT%\Middlewares\LoRa\Phy\" /Y

copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Utilities\utilities.c" "%PROJECT_ROOT%\Middlewares\LoRa\Utilities\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Utilities\utilities.h" "%PROJECT_ROOT%\Middlewares\LoRa\Utilities\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Utilities\timeServer.c" "%PROJECT_ROOT%\Middlewares\LoRa\Utilities\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Utilities\timeServer.h" "%PROJECT_ROOT%\Middlewares\LoRa\Utilities\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Utilities\delay.h" "%PROJECT_ROOT%\Middlewares\LoRa\Utilities\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Utilities\low_power.h" "%PROJECT_ROOT%\Middlewares\LoRa\Utilities\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Utilities\trace.h" "%PROJECT_ROOT%\Middlewares\LoRa\Utilities\" /Y

copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Crypto\aes.c" "%PROJECT_ROOT%\Middlewares\LoRa\Crypto\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Crypto\aes.h" "%PROJECT_ROOT%\Middlewares\LoRa\Crypto\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Crypto\cmac.c" "%PROJECT_ROOT%\Middlewares\LoRa\Crypto\" /Y
copy "%SOURCE_ROOT%\Middlewares\Third_Party\Lora\Crypto\cmac.h" "%PROJECT_ROOT%\Middlewares\LoRa\Crypto\" /Y

echo.
echo ====================================
echo File copy complete!
echo ====================================
echo.
echo Next steps:
echo 1. Refresh your project in STM32CubeIDE (F5)
echo 2. Clean the project
echo 3. Rebuild all
echo.
pause

