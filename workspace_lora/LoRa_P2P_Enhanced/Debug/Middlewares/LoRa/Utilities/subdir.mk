################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Utilities/timeServer.c \
C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Utilities/utilities.c 

OBJS += \
./Middlewares/LoRa/Utilities/timeServer.o \
./Middlewares/LoRa/Utilities/utilities.o 

C_DEPS += \
./Middlewares/LoRa/Utilities/timeServer.d \
./Middlewares/LoRa/Utilities/utilities.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/LoRa/Utilities/timeServer.o: C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Utilities/timeServer.c Middlewares/LoRa/Utilities/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32L072xx -c -I../Core/Inc -I../Application/User/Inc -I../Middlewares/LoRa/Phy -I../Middlewares/LoRa/Utilities -I../Middlewares/LoRa/Crypto -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Core/Inc" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/STM32L0xx_HAL_Driver/Inc" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/CMSIS/Device/ST/STM32L0xx/Include" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/CMSIS/Include" -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Middlewares" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/LoRa/Utilities/timeServer.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Middlewares/LoRa/Utilities/utilities.o: C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Middlewares/Third_Party/Lora/Utilities/utilities.c Middlewares/LoRa/Utilities/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32L072xx -c -I../Core/Inc -I../Application/User/Inc -I../Middlewares/LoRa/Phy -I../Middlewares/LoRa/Utilities -I../Middlewares/LoRa/Crypto -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Core/Inc" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/STM32L0xx_HAL_Driver/Inc" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/CMSIS/Device/ST/STM32L0xx/Include" -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Drivers/CMSIS/Include" -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/Wagne/OneDrive/Programmierungen/LORA P2P/LT-22222-L/workspace_lora/LoRa_P2P_Enhanced/Middlewares" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/LoRa/Utilities/utilities.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middlewares-2f-LoRa-2f-Utilities

clean-Middlewares-2f-LoRa-2f-Utilities:
	-$(RM) ./Middlewares/LoRa/Utilities/timeServer.cyclo ./Middlewares/LoRa/Utilities/timeServer.d ./Middlewares/LoRa/Utilities/timeServer.o ./Middlewares/LoRa/Utilities/timeServer.su ./Middlewares/LoRa/Utilities/utilities.cyclo ./Middlewares/LoRa/Utilities/utilities.d ./Middlewares/LoRa/Utilities/utilities.o ./Middlewares/LoRa/Utilities/utilities.su

.PHONY: clean-Middlewares-2f-LoRa-2f-Utilities

