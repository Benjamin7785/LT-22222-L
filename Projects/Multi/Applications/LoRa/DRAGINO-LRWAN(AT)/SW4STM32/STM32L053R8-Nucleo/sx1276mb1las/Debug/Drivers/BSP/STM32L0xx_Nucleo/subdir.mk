################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.c 

OBJS += \
./Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.o 

C_DEPS += \
./Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.o: C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.c Drivers/BSP/STM32L0xx_Nucleo/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L053xx -DUSE_STM32L0XX_NUCLEO -DUSE_HAL_DRIVER -DREGION_EU868 -c -I../../../../inc -I../../../../../../../../../Drivers/BSP/STM32L0xx_Nucleo -I../../../../../../../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../../../../../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../../../../../../../Drivers/CMSIS/Include -I../../../../../../../../../Middlewares/Third_Party/Lora/Crypto -I../../../../../../../../../Middlewares/Third_Party/Lora/Mac -I../../../../../../../../../Middlewares/Third_Party/Lora/Phy -I../../../../../../../../../Middlewares/Third_Party/Lora/Utilities -I../../../../../../../../../Middlewares/Third_Party/Lora/Core -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A1 -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A2 -I../../../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../../../Drivers/BSP/Components/hts221 -I../../../../../../../../../Drivers/BSP/Components/lps22hb -I../../../../../../../../../Drivers/BSP/Components/lps25hb -I../../../../../../../../../Drivers/BSP/Components/sx1276 -I../../../../../../../../../Drivers/BSP/sx1276mb1las -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-STM32L0xx_Nucleo

clean-Drivers-2f-BSP-2f-STM32L0xx_Nucleo:
	-$(RM) ./Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.cyclo ./Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.d ./Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.o ./Drivers/BSP/STM32L0xx_Nucleo/stm32l0xx_nucleo.su

.PHONY: clean-Drivers-2f-BSP-2f-STM32L0xx_Nucleo

