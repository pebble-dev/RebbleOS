TARGET:=test
# TOOLCHAIN_PATH:=~/yagarto-4.7.2/bin
TOOLCHAIN_PATH:=/Users/wang/gcc-arm-none-eabi/bin
TOOLCHAIN_PREFIX:=arm-none-eabi

# Optimization level, can be [0, 1, 2, 3, s].
OPTLVL:=0
DBG:=-g

STARTUP:=$(CURDIR)/hardware
LINKER_SCRIPT:=$(CURDIR)/Utilities/stm32_flash.ld

INCLUDE=-I$(CURDIR)/hardware
INCLUDE+=-I$(CURDIR)/Libraries/CMSIS/Device/ST/STM32F4xx/Include
INCLUDE+=-I$(CURDIR)/Libraries/CMSIS/Include
INCLUDE+=-I$(CURDIR)/Libraries/STM32F4xx_StdPeriph_Driver/inc

INCLUDE+=-I$(CURDIR)/config

# PBC
#INCLUDE+=-I/Users/wang/Documents/workspace/drone/firmware/Libraries/encryption/pbc
#INCLUDE+=-I/Users/wang/Documents/workspace/drone/firmware/Libraries/encryption/pbc/include

BUILD_DIR = $(CURDIR)/build
BIN_DIR = $(CURDIR)/binary

# vpath is used so object files are written to the current directory instead
# of the same directory as their source files
vpath %.c $(CURDIR)/Libraries/STM32F4xx_StdPeriph_Driver/src \
	  $(CURDIR)/Libraries/syscall $(CURDIR)/hardware

#	  /Users/wang/Documents/workspace/drone/firmware/Libraries/encryption/pbc/arith \
#	  $(CURDIR)/Libraries/encryption/gmp \
#	  /Users/wang/Documents/workspace/drone/firmware/Libraries/encryption/pbc/ecc \
#	  /Users/wang/Documents/workspace/drone/firmware/Libraries/encryption/pbc/misc

vpath %.s $(STARTUP)
ASRC=startup_stm32f4xx.s

# Project Source Files
SRC+=stm32f4xx_it.c
SRC+=system_stm32f4xx.c
SRC+=main.c

# Standard Peripheral Source Files
# SRC+=stm32f4xx_syscfg.c
SRC+=misc.c
#SRC+=stm32f4xx_adc.c
#SRC+=stm32f4xx_dac.c
# SRC+=stm32f4xx_dma.c
#SRC+=stm32f4xx_exti.c
#SRC+=stm32f4xx_flash.c
SRC+=stm32f4xx_gpio.c
SRC+=stm32f4xx_i2c.c
SRC+=stm32f4xx_rcc.c
#SRC+=stm32f4xx_spi.c
SRC+=stm32f4xx_tim.c
SRC+=stm32f4xx_usart.c
SRC+=stm32f4xx_rng.c

CDEFS=-DUSE_STDPERIPH_DRIVER
CDEFS+=-DSTM32F4XX
CDEFS+=-DHSE_VALUE=8000000

MCUFLAGS=-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
COMMONFLAGS=-O$(OPTLVL) $(DBG) -Wall --specs=nano.specs
CFLAGS=$(COMMONFLAGS) $(MCUFLAGS) $(INCLUDE) $(CDEFS)

LDLIBS=-lm
LDFLAGS=$(COMMONFLAGS) -fno-exceptions -ffunction-sections -fdata-sections -nostartfiles -Wl,--gc-sections,-T$(LINKER_SCRIPT) -mthumb -v

OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)

CC=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-gcc
LD=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-gcc
OBJCOPY=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-objcopy
AS=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-as
AR=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-ar
GDB=$(TOOLCHAIN_PATH)/$(TOOLCHAIN_PREFIX)-gdb

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

all: $(OBJ)
	$(AS) -o $(ASRC:%.s=$(BUILD_DIR)/%.o) $(STARTUP)/$(ASRC)
	$(CC) -o $(BIN_DIR)/$(TARGET).elf $(LDFLAGS) $(OBJ) $(ASRC:%.s=$(BUILD_DIR)/%.o) $(LDLIBS)
	$(OBJCOPY) -O ihex $(BIN_DIR)/$(TARGET).elf $(BIN_DIR)/$(TARGET).hex
	$(OBJCOPY) -O binary $(BIN_DIR)/$(TARGET).elf $(BIN_DIR)/$(TARGET).bin

.PHONY: clean

clean:
	rm -f $(OBJ)
	rm -f $(ASRC:%.s=$(BUILD_DIR)/%.o)
	rm -f $(BIN_DIR)/$(TARGET).elf
	rm -f $(BIN_DIR)/$(TARGET).hex
	rm -f $(BIN_DIR)/$(TARGET).bin


flash: all
	st-flash write $(BIN_DIR)/$(TARGET).bin 0x8000000

reflash: clean flash
