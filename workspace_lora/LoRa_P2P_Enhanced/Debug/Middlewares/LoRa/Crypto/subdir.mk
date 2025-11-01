################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Crypto/aes.c \
C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Crypto/cmac.c 

OBJS += \
./Middlewares/LoRa/Crypto/aes.o \
./Middlewares/LoRa/Crypto/cmac.o 

C_DEPS += \
./Middlewares/LoRa/Crypto/aes.d \
./Middlewares/LoRa/Crypto/cmac.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/LoRa/Crypto/aes.o: C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Crypto/aes.c Middlewares/LoRa/Crypto/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32L072xx -c -I../Core/Inc -I../Application/User/Inc -I../Middlewares/LoRa/Phy -I../Middlewares/LoRa/Utilities -I../Middlewares/LoRa/Crypto -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Core/Inc" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/STM32L0xx_HAL_Driver/Inc" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/CMSIS/Device/ST/STM32L0xx/Include" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/CMSIS/Include" -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Middlewares" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/LoRa/Crypto/aes.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Middlewares/LoRa/Crypto/cmac.o: C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Crypto/cmac.c Middlewares/LoRa/Crypto/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32L072xx -c -I../Core/Inc -I../Application/User/Inc -I../Middlewares/LoRa/Phy -I../Middlewares/LoRa/Utilities -I../Middlewares/LoRa/Crypto -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Core/Inc" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/STM32L0xx_HAL_Driver/Inc" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/CMSIS/Device/ST/STM32L0xx/Include" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/CMSIS/Include" -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Middlewares" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/LoRa/Crypto/cmac.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middlewares-2f-LoRa-2f-Crypto

clean-Middlewares-2f-LoRa-2f-Crypto:
	-$(RM) ./Middlewares/LoRa/Crypto/aes.cyclo ./Middlewares/LoRa/Crypto/aes.d ./Middlewares/LoRa/Crypto/aes.o ./Middlewares/LoRa/Crypto/aes.su ./Middlewares/LoRa/Crypto/cmac.cyclo ./Middlewares/LoRa/Crypto/cmac.d ./Middlewares/LoRa/Crypto/cmac.o ./Middlewares/LoRa/Crypto/cmac.su

.PHONY: clean-Middlewares-2f-LoRa-2f-Crypto

