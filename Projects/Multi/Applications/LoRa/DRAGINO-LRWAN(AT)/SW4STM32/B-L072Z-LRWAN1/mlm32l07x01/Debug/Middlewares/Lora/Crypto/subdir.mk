################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Crypto/aes.c \
C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Crypto/cmac.c 

OBJS += \
./Middlewares/Lora/Crypto/aes.o \
./Middlewares/Lora/Crypto/cmac.o 

C_DEPS += \
./Middlewares/Lora/Crypto/aes.d \
./Middlewares/Lora/Crypto/cmac.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Lora/Crypto/aes.o: C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Crypto/aes.c Middlewares/Lora/Crypto/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L072xx -DUSE_B_L072Z_LRWAN1 -DUSE_HAL_DRIVER -DREGION_EU868 -c -I../../../../inc -I../../../../../../../../../Drivers/BSP/MLM32L07X01 -I../../../../../../../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../../../../../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../../../../../../../Drivers/CMSIS/Include -I../../../../../../../../../Middlewares/Third_Party/Lora/Crypto -I../../../../../../../../../Middlewares/Third_Party/Lora/Mac -I../../../../../../../../../Middlewares/Third_Party/Lora/Phy -I../../../../../../../../../Middlewares/Third_Party/Lora/Utilities -I../../../../../../../../../Middlewares/Third_Party/Lora/Core -I../../../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../../../Drivers/BSP/Components/hts221 -I../../../../../../../../../Drivers/BSP/Components/lps22hb -I../../../../../../../../../Drivers/BSP/Components/lps25hb -I../../../../../../../../../Drivers/BSP/Components/sx1276 -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A1 -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A2 -I../../../../../../../../../Drivers/BSP/B-L072Z-LRWAN1 -I../../../../../../../../../Middlewares/Third_Party/Lora/Mac/region -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/Lora/Crypto/aes.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Middlewares/Lora/Crypto/cmac.o: C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Crypto/cmac.c Middlewares/Lora/Crypto/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L072xx -DUSE_B_L072Z_LRWAN1 -DUSE_HAL_DRIVER -DREGION_EU868 -c -I../../../../inc -I../../../../../../../../../Drivers/BSP/MLM32L07X01 -I../../../../../../../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../../../../../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../../../../../../../Drivers/CMSIS/Include -I../../../../../../../../../Middlewares/Third_Party/Lora/Crypto -I../../../../../../../../../Middlewares/Third_Party/Lora/Mac -I../../../../../../../../../Middlewares/Third_Party/Lora/Phy -I../../../../../../../../../Middlewares/Third_Party/Lora/Utilities -I../../../../../../../../../Middlewares/Third_Party/Lora/Core -I../../../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../../../Drivers/BSP/Components/hts221 -I../../../../../../../../../Drivers/BSP/Components/lps22hb -I../../../../../../../../../Drivers/BSP/Components/lps25hb -I../../../../../../../../../Drivers/BSP/Components/sx1276 -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A1 -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A2 -I../../../../../../../../../Drivers/BSP/B-L072Z-LRWAN1 -I../../../../../../../../../Middlewares/Third_Party/Lora/Mac/region -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/Lora/Crypto/cmac.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middlewares-2f-Lora-2f-Crypto

clean-Middlewares-2f-Lora-2f-Crypto:
	-$(RM) ./Middlewares/Lora/Crypto/aes.cyclo ./Middlewares/Lora/Crypto/aes.d ./Middlewares/Lora/Crypto/aes.o ./Middlewares/Lora/Crypto/aes.su ./Middlewares/Lora/Crypto/cmac.cyclo ./Middlewares/Lora/Crypto/cmac.d ./Middlewares/Lora/Crypto/cmac.o ./Middlewares/Lora/Crypto/cmac.su

.PHONY: clean-Middlewares-2f-Lora-2f-Crypto

