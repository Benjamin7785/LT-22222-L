################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Projects/Multi/Applications/LoRa/DRAGINO-LRWAN(AT)/SW4STM32/STM32L073RZ-Nucleo/startup_stm32l073xx.s 

OBJS += \
./Projects/SW4STM32/startup_stm32l073xx.o 

S_DEPS += \
./Projects/SW4STM32/startup_stm32l073xx.d 


# Each subdirectory must supply rules for building sources it contributes
Projects/SW4STM32/startup_stm32l073xx.o: C:/Users/Wagne/OneDrive/Programmierungen/LORA\ P2P/LT-22222-L/Projects/Multi/Applications/LoRa/DRAGINO-LRWAN(AT)/SW4STM32/STM32L073RZ-Nucleo/startup_stm32l073xx.s Projects/SW4STM32/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m0plus -g3 -c -x assembler-with-cpp -MMD -MP -MF"Projects/SW4STM32/startup_stm32l073xx.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"

clean: clean-Projects-2f-SW4STM32

clean-Projects-2f-SW4STM32:
	-$(RM) ./Projects/SW4STM32/startup_stm32l073xx.d ./Projects/SW4STM32/startup_stm32l073xx.o

.PHONY: clean-Projects-2f-SW4STM32

