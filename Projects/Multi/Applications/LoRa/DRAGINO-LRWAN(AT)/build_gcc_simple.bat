@echo off
REM ============================================
REM Simple GCC Build (No Make Required!)
REM ============================================

echo.
echo ============================================
echo Building LoRa P2P Enhanced with GCC
echo (No Make required!)
echo ============================================
echo.

REM Check ARM GCC
where arm-none-eabi-gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: ARM GCC not found!
    echo.
    echo Download from: https://developer.arm.com/downloads/-/gnu-rm
    echo Install and add to PATH.
    echo.
    pause
    exit /b 1
)

echo ARM GCC found!
arm-none-eabi-gcc --version | findstr "gcc"
echo.

REM Create build directory
if not exist build mkdir build

echo Compiling source files...
echo.

REM Common flags
set MCU=-mcpu=cortex-m0plus -mthumb -mfloat-abi=soft
set DEFS=-DSTM32L072xx -DUSE_HAL_DRIVER
set OPT=-Os
set WARNINGS=-Wall
set INCLUDES=-Iinc -I../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy
set INCLUDES=%INCLUDES% -I../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../Drivers/CMSIS/Include
set INCLUDES=%INCLUDES% -I../../Drivers/BSP/MLM32L0XX -I../../Drivers/BSP/Components/sx1276mb1las
set INCLUDES=%INCLUDES% -I../../Drivers/BSP/Components/sx1276 -I../../Drivers/BSP/Components/relay_output
set INCLUDES=%INCLUDES% -I../../Drivers/BSP/Components/npn_output -I../../Drivers/BSP/Components/gpio_exti
set INCLUDES=%INCLUDES% -I../../Drivers/BSP/Components/flash_eraseprogram -I../../Drivers/BSP/Components/iwdg
set INCLUDES=%INCLUDES% -I../../Middlewares/Third_Party/Lora/Phy -I../../Middlewares/Third_Party/Lora/Utilities
set INCLUDES=%INCLUDES% -I../../Middlewares/Third_Party/Lora/Crypto

set CFLAGS=%MCU% %DEFS% %OPT% %WARNINGS% %INCLUDES% -ffunction-sections -fdata-sections -g

echo [1/55] Compiling main.c...
arm-none-eabi-gcc %CFLAGS% -c src/main.c -o build/main.o

echo [2/55] Compiling at.c...
arm-none-eabi-gcc %CFLAGS% -c src/at.c -o build/at.o

echo [3/55] Compiling command.c...
arm-none-eabi-gcc %CFLAGS% -c src/command.c -o build/command.o

echo [4/55] Compiling lora.c...
arm-none-eabi-gcc %CFLAGS% -c src/lora.c -o build/lora.o

echo [5/55] Compiling test_rf.c...
arm-none-eabi-gcc %CFLAGS% -c src/test_rf.c -o build/test_rf.o

echo [6/55] Compiling vcom.c...
arm-none-eabi-gcc %CFLAGS% -c src/vcom.c -o build/vcom.o

echo [7/55] Compiling debug.c...
arm-none-eabi-gcc %CFLAGS% -c src/debug.c -o build/debug.o

echo [8/55] Compiling hw_gpio.c...
arm-none-eabi-gcc %CFLAGS% -c src/hw_gpio.c -o build/hw_gpio.o

echo [9/55] Compiling hw_rtc.c...
arm-none-eabi-gcc %CFLAGS% -c src/hw_rtc.c -o build/hw_rtc.o

echo [10/55] Compiling hw_spi.c...
arm-none-eabi-gcc %CFLAGS% -c src/hw_spi.c -o build/hw_spi.o

echo [11/55] Compiling utilities...
arm-none-eabi-gcc %CFLAGS% -c src/tiny_sscanf.c -o build/tiny_sscanf.o
arm-none-eabi-gcc %CFLAGS% -c src/tiny_vsnprintf.c -o build/tiny_vsnprintf.o

echo [12/55] Compiling HAL/board specific...
arm-none-eabi-gcc %CFLAGS% -c src/stm32l0xx_hal_msp.c -o build/stm32l0xx_hal_msp.o
arm-none-eabi-gcc %CFLAGS% -c src/stm32l0xx_hw.c -o build/stm32l0xx_hw.o
arm-none-eabi-gcc %CFLAGS% -c src/stm32l0xx_it.c -o build/stm32l0xx_it.o

echo [15/55] Compiling HAL drivers...
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal.c -o build/stm32l0xx_hal.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc.c -o build/stm32l0xx_hal_adc.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_cortex.c -o build/stm32l0xx_hal_cortex.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dma.c -o build/stm32l0xx_hal_dma.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash.c -o build/stm32l0xx_hal_flash.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash_ex.c -o build/stm32l0xx_hal_flash_ex.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_gpio.c -o build/stm32l0xx_hal_gpio.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c.c -o build/stm32l0xx_hal_i2c.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_iwdg.c -o build/stm32l0xx_hal_iwdg.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr.c -o build/stm32l0xx_hal_pwr.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr_ex.c -o build/stm32l0xx_hal_pwr_ex.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc.c -o build/stm32l0xx_hal_rcc.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc_ex.c -o build/stm32l0xx_hal_rcc_ex.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc.c -o build/stm32l0xx_hal_rtc.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc_ex.c -o build/stm32l0xx_hal_rtc_ex.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_spi.c -o build/stm32l0xx_hal_spi.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim.c -o build/stm32l0xx_hal_tim.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim_ex.c -o build/stm32l0xx_hal_tim_ex.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart.c -o build/stm32l0xx_hal_uart.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart_ex.c -o build/stm32l0xx_hal_uart_ex.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc_ex.c -o build/stm32l0xx_hal_adc_ex.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c_ex.c -o build/stm32l0xx_hal_i2c_ex.o

echo [35/55] Compiling BSP components...
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/BSP/MLM32L0XX/mlm32l0xx.c -o build/mlm32l0xx.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/BSP/Components/sx1276mb1las/sx1276mb1las.c -o build/sx1276mb1las.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/BSP/Components/sx1276/sx1276.c -o build/sx1276_bsp.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/BSP/Components/relay_output/relay_output.c -o build/relay_output.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/BSP/Components/npn_output/npn_output.c -o build/npn_output.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/BSP/Components/gpio_exti/gpio_exti.c -o build/gpio_exti.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/BSP/Components/flash_eraseprogram/flash_eraseprogram.c -o build/flash_eraseprogram.o
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/BSP/Components/iwdg/iwdg.c -o build/iwdg.o

echo [43/55] Compiling LoRa middleware...
arm-none-eabi-gcc %CFLAGS% -c ../../Middlewares/Third_Party/Lora/Phy/radio.c -o build/radio.o
arm-none-eabi-gcc %CFLAGS% -c ../../Middlewares/Third_Party/Lora/Phy/sx1276.c -o build/sx1276.o
arm-none-eabi-gcc %CFLAGS% -c ../../Middlewares/Third_Party/Lora/Utilities/delay.c -o build/delay.o
arm-none-eabi-gcc %CFLAGS% -c ../../Middlewares/Third_Party/Lora/Utilities/low_power_manager.c -o build/low_power_manager.o
arm-none-eabi-gcc %CFLAGS% -c ../../Middlewares/Third_Party/Lora/Utilities/timeServer.c -o build/timeServer.o
arm-none-eabi-gcc %CFLAGS% -c ../../Middlewares/Third_Party/Lora/Utilities/utilities.c -o build/utilities.o
arm-none-eabi-gcc %CFLAGS% -c ../../Middlewares/Third_Party/Lora/Utilities/trace.c -o build/trace.o
arm-none-eabi-gcc %CFLAGS% -c ../../Middlewares/Third_Party/Lora/Utilities/queue.c -o build/queue.o
arm-none-eabi-gcc %CFLAGS% -c ../../Middlewares/Third_Party/Lora/Crypto/aes.c -o build/aes.o
arm-none-eabi-gcc %CFLAGS% -c ../../Middlewares/Third_Party/Lora/Crypto/cmac.c -o build/cmac.o

echo [53/55] Compiling CMSIS...
arm-none-eabi-gcc %CFLAGS% -c ../../Drivers/CMSIS/Device/ST/STM32L0xx/Source/Templates/system_stm32l0xx.c -o build/system_stm32l0xx.o

echo [54/55] Assembling startup...
arm-none-eabi-gcc %MCU% -c -x assembler-with-cpp SW4STM32/B-L072Z-LRWAN1/startup_stm32l072xx.s -o build/startup_stm32l072xx.o

echo [55/55] Linking...
arm-none-eabi-gcc %MCU% -specs=nano.specs -TSW4STM32/B-L072Z-LRWAN1/mlm32l07x01/STM32L072CZYx_FLASH.ld -Wl,-Map=build/LoRa_P2P_Enhanced.map,--cref -Wl,--gc-sections build/*.o -lc -lm -lnosys -o build/LoRa_P2P_Enhanced.elf

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ============================================
    echo LINKING FAILED!
    echo ============================================
    pause
    exit /b 1
)

echo.
echo Creating HEX file...
arm-none-eabi-objcopy -O ihex build/LoRa_P2P_Enhanced.elf build/LoRa_P2P_Enhanced.hex

echo Creating BIN file...
arm-none-eabi-objcopy -O binary build/LoRa_P2P_Enhanced.elf build/LoRa_P2P_Enhanced.bin

echo.
echo ============================================
echo BUILD SUCCESSFUL!
echo ============================================
echo.
arm-none-eabi-size build/LoRa_P2P_Enhanced.elf
echo.
echo Output files:
echo   HEX: build\LoRa_P2P_Enhanced.hex
echo   BIN: build\LoRa_P2P_Enhanced.bin
echo   ELF: build\LoRa_P2P_Enhanced.elf
echo   MAP: build\LoRa_P2P_Enhanced.map
echo.
echo Ready to flash!
echo.
pause



