################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Mac/LoRaMac.c \
C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Mac/LoRaMacCrypto.c 

OBJS += \
./Middlewares/Lora/Mac/LoRaMac.o \
./Middlewares/Lora/Mac/LoRaMacCrypto.o 

C_DEPS += \
./Middlewares/Lora/Mac/LoRaMac.d \
./Middlewares/Lora/Mac/LoRaMacCrypto.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Lora/Mac/LoRaMac.o: C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Mac/LoRaMac.c Middlewares/Lora/Mac/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L073xx -DUSE_STM32L0XX_NUCLEO -DUSE_HAL_DRIVER -DREGION_EU868 -c -I../../../../inc -I../../../../../../../../../Drivers/BSP/STM32L0xx_Nucleo -I../../../../../../../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../../../../../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../../../../../../../Drivers/CMSIS/Include -I../../../../../../../../../Middlewares/Third_Party/Lora/Crypto -I../../../../../../../../../Middlewares/Third_Party/Lora/Mac -I../../../../../../../../../Middlewares/Third_Party/Lora/Phy -I../../../../../../../../../Middlewares/Third_Party/Lora/Utilities -I../../../../../../../../../Middlewares/Third_Party/Lora/Core -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A1 -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A2 -I../../../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../../../Drivers/BSP/Components/hts221 -I../../../../../../../../../Drivers/BSP/Components/lps22hb -I../../../../../../../../../Drivers/BSP/Components/lps25hb -I../../../../../../../../../Drivers/BSP/Components/sx1276 -I../../../../../../../../../Drivers/BSP/sx1276mb1las -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/Lora/Mac/LoRaMac.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Middlewares/Lora/Mac/LoRaMacCrypto.o: C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Mac/LoRaMacCrypto.c Middlewares/Lora/Mac/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L073xx -DUSE_STM32L0XX_NUCLEO -DUSE_HAL_DRIVER -DREGION_EU868 -c -I../../../../inc -I../../../../../../../../../Drivers/BSP/STM32L0xx_Nucleo -I../../../../../../../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../../../../../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../../../../../../../Drivers/CMSIS/Include -I../../../../../../../../../Middlewares/Third_Party/Lora/Crypto -I../../../../../../../../../Middlewares/Third_Party/Lora/Mac -I../../../../../../../../../Middlewares/Third_Party/Lora/Phy -I../../../../../../../../../Middlewares/Third_Party/Lora/Utilities -I../../../../../../../../../Middlewares/Third_Party/Lora/Core -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A1 -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A2 -I../../../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../../../Drivers/BSP/Components/hts221 -I../../../../../../../../../Drivers/BSP/Components/lps22hb -I../../../../../../../../../Drivers/BSP/Components/lps25hb -I../../../../../../../../../Drivers/BSP/Components/sx1276 -I../../../../../../../../../Drivers/BSP/sx1276mb1las -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/Lora/Mac/LoRaMacCrypto.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middlewares-2f-Lora-2f-Mac

clean-Middlewares-2f-Lora-2f-Mac:
	-$(RM) ./Middlewares/Lora/Mac/LoRaMac.cyclo ./Middlewares/Lora/Mac/LoRaMac.d ./Middlewares/Lora/Mac/LoRaMac.o ./Middlewares/Lora/Mac/LoRaMac.su ./Middlewares/Lora/Mac/LoRaMacCrypto.cyclo ./Middlewares/Lora/Mac/LoRaMacCrypto.d ./Middlewares/Lora/Mac/LoRaMacCrypto.o ./Middlewares/Lora/Mac/LoRaMacCrypto.su

.PHONY: clean-Middlewares-2f-Lora-2f-Mac

