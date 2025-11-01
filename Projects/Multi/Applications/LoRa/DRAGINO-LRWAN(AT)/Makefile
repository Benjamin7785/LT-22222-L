######################################
# target
######################################
TARGET = LoRa_P2P_Enhanced

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Os

#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
src/main.c \
src/at.c \
src/command.c \
src/lora.c \
src/test_rf.c \
src/vcom.c \
src/debug.c \
src/hw_gpio.c \
src/hw_rtc.c \
src/hw_spi.c \
src/tiny_sscanf.c \
src/tiny_vsnprintf.c \
src/stm32l0xx_hal_msp.c \
src/stm32l0xx_hw.c \
src/stm32l0xx_it.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc_ex.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_cortex.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dma.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash_ex.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_gpio.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c_ex.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_iwdg.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr_ex.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc_ex.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc_ex.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_spi.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim_ex.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart.c \
../../Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart_ex.c \
../../Drivers/BSP/MLM32L0XX/mlm32l0xx.c \
../../Drivers/BSP/Components/sx1276mb1las/sx1276mb1las.c \
../../Drivers/BSP/Components/sx1276/sx1276.c \
../../Drivers/BSP/Components/relay_output/relay_output.c \
../../Drivers/BSP/Components/npn_output/npn_output.c \
../../Drivers/BSP/Components/gpio_exti/gpio_exti.c \
../../Drivers/BSP/Components/flash_eraseprogram/flash_eraseprogram.c \
../../Drivers/BSP/Components/iwdg/iwdg.c \
../../Middlewares/Third_Party/Lora/Phy/radio.c \
../../Middlewares/Third_Party/Lora/Phy/sx1276.c \
../../Middlewares/Third_Party/Lora/Utilities/delay.c \
../../Middlewares/Third_Party/Lora/Utilities/low_power_manager.c \
../../Middlewares/Third_Party/Lora/Utilities/timeServer.c \
../../Middlewares/Third_Party/Lora/Utilities/utilities.c \
../../Middlewares/Third_Party/Lora/Utilities/trace.c \
../../Middlewares/Third_Party/Lora/Utilities/queue.c \
../../Middlewares/Third_Party/Lora/Crypto/aes.c \
../../Middlewares/Third_Party/Lora/Crypto/cmac.c \
../../Drivers/CMSIS/Device/ST/STM32L0xx/Source/Templates/system_stm32l0xx.c

# ASM sources
ASM_SOURCES =  \
SW4STM32/B-L072Z-LRWAN1/startup_stm32l072xx.s

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m0plus

# fpu
# NONE for Cortex-M0+

# float-abi
FLOAT-ABI = -mfloat-abi=soft

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DSTM32L072xx \
-DUSE_HAL_DRIVER

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-Iinc \
-I../../Drivers/STM32L0xx_HAL_Driver/Inc \
-I../../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy \
-I../../Drivers/CMSIS/Device/ST/STM32L0xx/Include \
-I../../Drivers/CMSIS/Include \
-I../../Drivers/BSP/MLM32L0XX \
-I../../Drivers/BSP/Components/sx1276mb1las \
-I../../Drivers/BSP/Components/sx1276 \
-I../../Drivers/BSP/Components/relay_output \
-I../../Drivers/BSP/Components/npn_output \
-I../../Drivers/BSP/Components/gpio_exti \
-I../../Drivers/BSP/Components/flash_eraseprogram \
-I../../Drivers/BSP/Components/iwdg \
-I../../Middlewares/Third_Party/Lora/Phy \
-I../../Middlewares/Third_Party/Lora/Utilities \
-I../../Middlewares/Third_Party/Lora/Crypto

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = SW4STM32/B-L072Z-LRWAN1/mlm32l07x01/STM32L072CZYx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@echo "Compiling $<"
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "Assembling $<"
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "Linking..."
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo ""
	@echo "===== Build Complete ====="
	$(SZ) $@
	@echo "=========================="

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "Creating HEX file..."
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "Creating BIN file..."
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# flash
#######################################
flash: all
	@echo "Flashing to target..."
	STM32_Programmer_CLI.exe -c port=SWD -w $(BUILD_DIR)/$(TARGET).hex -v -rst

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***


