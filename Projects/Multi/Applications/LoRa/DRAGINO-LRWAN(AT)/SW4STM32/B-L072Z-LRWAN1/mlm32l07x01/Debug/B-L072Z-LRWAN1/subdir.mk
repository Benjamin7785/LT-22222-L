################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Drivers/BSP/B-L072Z-LRWAN1/b-l072z-lrwan1.c 

OBJS += \
./B-L072Z-LRWAN1/b-l072z-lrwan1.o 

C_DEPS += \
./B-L072Z-LRWAN1/b-l072z-lrwan1.d 


# Each subdirectory must supply rules for building sources it contributes
B-L072Z-LRWAN1/b-l072z-lrwan1.o: C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Drivers/BSP/B-L072Z-LRWAN1/b-l072z-lrwan1.c B-L072Z-LRWAN1/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L072xx -DUSE_B_L072Z_LRWAN1 -DUSE_HAL_DRIVER -DREGION_EU868 -c -I../../../../inc -I../../../../../../../../../Drivers/BSP/MLM32L07X01 -I../../../../../../../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../../../../../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../../../../../../../Drivers/CMSIS/Include -I../../../../../../../../../Middlewares/Third_Party/Lora/Crypto -I../../../../../../../../../Middlewares/Third_Party/Lora/Mac -I../../../../../../../../../Middlewares/Third_Party/Lora/Phy -I../../../../../../../../../Middlewares/Third_Party/Lora/Utilities -I../../../../../../../../../Middlewares/Third_Party/Lora/Core -I../../../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../../../Drivers/BSP/Components/hts221 -I../../../../../../../../../Drivers/BSP/Components/lps22hb -I../../../../../../../../../Drivers/BSP/Components/lps25hb -I../../../../../../../../../Drivers/BSP/Components/sx1276 -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A1 -I../../../../../../../../../Drivers/BSP/X_NUCLEO_IKS01A2 -I../../../../../../../../../Drivers/BSP/B-L072Z-LRWAN1 -I../../../../../../../../../Middlewares/Third_Party/Lora/Mac/region -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"B-L072Z-LRWAN1/b-l072z-lrwan1.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-B-2d-L072Z-2d-LRWAN1

clean-B-2d-L072Z-2d-LRWAN1:
	-$(RM) ./B-L072Z-LRWAN1/b-l072z-lrwan1.cyclo ./B-L072Z-LRWAN1/b-l072z-lrwan1.d ./B-L072Z-LRWAN1/b-l072z-lrwan1.o ./B-L072Z-LRWAN1/b-l072z-lrwan1.su

.PHONY: clean-B-2d-L072Z-2d-LRWAN1

