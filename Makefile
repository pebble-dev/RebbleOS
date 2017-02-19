TARGET:=RebbleOS
# TODO change to your ARM gcc toolchain path
TOOLCHAIN_ROOT:=
TOOLCHAIN_PATH:=
TOOLCHAIN_PREFIX:=arm-none-eabi

# Optimization level, can be [0, 1, 2, 3, s].
OPTLVL:=0
DBG:=-g

FREERTOS:=$(CURDIR)/FreeRTOS
STARTUP:=$(CURDIR)/Hardware
LINKER_SCRIPT:=$(CURDIR)/Utilities/stm32_flash.ld

INCLUDE=-I$(CURDIR)/Hardware
INCLUDE+=-I$(CURDIR)/Hardware/Pebble_Snowy
INCLUDE+=-I$(FREERTOS)/include
INCLUDE+=-I$(FREERTOS)/portable/GCC/ARM_CM4F
INCLUDE+=-I$(CURDIR)/Platform/CMSIS/Device/ST/STM32F4xx/Include
INCLUDE+=-I$(CURDIR)/Platform/CMSIS/Include
INCLUDE+=-I$(CURDIR)/Platform/STM32F4xx_StdPeriph_Driver/inc
INCLUDE+=-I$(CURDIR)/Libraries/UGUI
INCLUDE+=-I$(CURDIR)/Libraries/neographics/src/
INCLUDE+=-I$(CURDIR)/Libraries/neographics/src/draw_command
INCLUDE+=-I$(CURDIR)/Libraries/neographics/src/path
INCLUDE+=-I$(CURDIR)/Libraries/neographics/src/primitives
INCLUDE+=-I$(CURDIR)/Libraries/neographics/src/types
INCLUDE+=-I$(CURDIR)/Libraries/neographics/src/fonts
INCLUDE+=-I$(CURDIR)/Libraries/neographics/src/text
INCLUDE+=-I$(CURDIR)/Watchfaces
INCLUDE+=-I$(CURDIR)/Config
INCLUDE+=-I$(CURDIR)/RebbleOS
INCLUDE+=-I$(CURDIR)/RebbleOS/Gui
INCLUDE+=-I$(CURDIR)/libRebbleOS

BUILD_DIR = $(CURDIR)/build
BIN_DIR = $(CURDIR)/binary

# vpath is used so object files are written to the current directory instead
# of the same directory as their source files
vpath %.c $(CURDIR)/Platform/STM32F4xx_StdPeriph_Driver/src \
          $(CURDIR)/Libraries/syscall \
          $(CURDIR)/Hardware \
          $(CURDIR)/Hardware/Pebble_Snowy \
          $(CURDIR)/RebbleOS \
          $(CURDIR)/RebbleOS/Gui \
          $(CURDIR)/libRebbleOS \
          $(CURDIR)/libRebbleOS \
          $(CURDIR)/Watchfaces \
          $(FREERTOS) \
          $(CURDIR)/Libraries/UGUI \
          $(CURDIR)/Libraries/neographics/src \
          $(CURDIR)/Libraries/neographics/src/draw_command \
          $(CURDIR)/Libraries/neographics/src/path \
          $(CURDIR)/Libraries/neographics/src/primitives \
          $(CURDIR)/Libraries/neographics/src/fonts \
          $(CURDIR)/Libraries/neographics/src/text \
          $(FREERTOS)/portable/MemMang $(FREERTOS)/portable/GCC/ARM_CM4F 

vpath %.s $(STARTUP)
ASRC=startup_stm32f4xx.s

# Project Source Files
SRC+=stm32f4xx_it.c
SRC+=system_stm32f4xx.c
SRC+=syscalls.c

# FreeRTOS Source Files
SRC+=port.c
SRC+=list.c
SRC+=queue.c
SRC+=tasks.c
SRC+=event_groups.c
SRC+=timers.c
SRC+=heap_4.c

# Standard Peripheral Source Files
SRC+=misc.c
SRC+=stm32f4xx_dcmi.c
#SRC+=stm32f4xx_hash.c
SRC+=stm32f4xx_rtc.c
SRC+=stm32f4xx_adc.c
SRC+=stm32f4xx_dma.c
#SRC+=stm32f4xx_hash_md5.c
SRC+=stm32f4xx_sai.c
SRC+=stm32f4xx_can.c
SRC+=stm32f4xx_dma2d.c
#SRC+=stm32f4xx_hash_sha1.c
SRC+=stm32f4xx_sdio.c
SRC+=stm32f4xx_cec.c
SRC+=stm32f4xx_dsi.c
SRC+=stm32f4xx_i2c.c
SRC+=stm32f4xx_spdifrx.c
SRC+=stm32f4xx_crc.c
SRC+=stm32f4xx_exti.c
SRC+=stm32f4xx_iwdg.c
SRC+=stm32f4xx_spi.c
#SRC+=stm32f4xx_cryp.c
SRC+=stm32f4xx_flash.c
SRC+=stm32f4xx_lptim.c
SRC+=stm32f4xx_syscfg.c
#SRC+=stm32f4xx_cryp_aes.c
SRC+=stm32f4xx_flash_ramfunc.c
SRC+=stm32f4xx_ltdc.c
SRC+=stm32f4xx_tim.c
#SRC+=stm32f4xx_cryp_des.c
#SRC+=stm32f4xx_fmc.c
SRC+=stm32f4xx_pwr.c
SRC+=stm32f4xx_usart.c
#SRC+=stm32f4xx_cryp_tdes.c
SRC+=stm32f4xx_fmpi2c.c
SRC+=stm32f4xx_qspi.c
SRC+=stm32f4xx_wwdg.c
SRC+=stm32f4xx_dac.c
#SRC+=stm32f4xx_fsmc.c
SRC+=stm32f4xx_rcc.c
SRC+=stm32f4xx_dbgmcu.c
SRC+=stm32f4xx_gpio.c
SRC+=stm32f4xx_rng.c

# std Libraries
SRC+=stdarg.c

# uGUI
SRC+=ugui.c

# NeoGraphics
SRC+=common.c
SRC+=context.c
SRC+=draw_command.c
SRC+=path.c
SRC+=circle.c
SRC+=line.c
SRC+=rect.c
SRC+=text.c
SRC+=fonts.c

# neographics / RebbleOS
SRC+=neographics.c

# drivers etc
SRC+=stm32f4x_i2c.c

# Pebble hardware
SRC+=Resources/FPGA_4.3_snowy.o
SRC+=snowy_display.c
SRC+=snowy_backlight.c
SRC+=snowy_buttons.c
SRC+=snowy_power.c
SRC+=snowy_rtc.c
SRC+=snowy_scanlines.c
SRC+=snowy_vibrate.c
SRC+=snowy_ambient.c
SRC+=snowy.c

# RebbleOS
SRC+=appmanager.c
SRC+=rebbleos.c
SRC+=ambient.c
SRC+=backlight.c
SRC+=buttons.c
SRC+=display.c
SRC+=gyro.c
SRC+=main.c
SRC+=power.c
SRC+=smartstrap.c
SRC+=rebble_time.c
SRC+=vibrate.c
SRC+=rebble_memory.c

# libRebbleOS
SRC+=librebble.c
SRC+=window.c
SRC+=layer.c
SRC+=text_layer.c
SRC+=math_sin.c

# watchface test
SRC+=simple.c

#RebbleOS Gui
SRC+=gui.c
SRC+=menu.c

CDEFS=-DUSE_STDPERIPH_DRIVER
CDEFS+=-DSTM32F4XX
#CDEFS+=-DSTM32F40_41xxx
CDEFS+=-DSTM32F429_439xx
CDEFS+=-DHSI_VALUE=16000000
CDEFS+=-D__FPU_PRESENT=1
CDEFS+=-D__FPU_USED=1
CDEFS+=-DARM_MATH_CM4

MCUFLAGS=-mcpu=cortex-m4 -mthumb -mlittle-endian -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -fsingle-precision-constant -finline-functions -std=gnu99 -fomit-frame-pointer -falign-functions=16
COMMONFLAGS=-O$(OPTLVL) $(DBG) -Wall -ffunction-sections -fdata-sections
CFLAGS=$(COMMONFLAGS) $(MCUFLAGS) $(INCLUDE) $(CDEFS)

LDLIBS=-lm -lc -lgcc
LDFLAGS=$(MCUFLAGS) -u _scanf_float -u _printf_float -fno-exceptions -Wl,--gc-sections,-T$(LINKER_SCRIPT),-Map,$(BIN_DIR)/$(TARGET).map
#LDFLAGS=$(MCUFLAGS) -T$(LINKER_SCRIPT) -specs=nosys.specs

CC=$(TOOLCHAIN_PREFIX)-gcc
LD=/$(TOOLCHAIN_PREFIX)-gcc
OBJCOPY=$(TOOLCHAIN_PREFIX)-objcopy
AS=$(TOOLCHAIN_PREFIX)-as
AR=$(TOOLCHAIN_PREFIX)-ar
GDB=$(TOOLCHAIN_PREFIX)-gdb

OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: %.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

all: $(OBJ) fpga
	@echo [AS] $(ASRC)
	@$(AS) -o $(ASRC:%.s=$(BUILD_DIR)/%.o) $(STARTUP)/$(ASRC)
	@echo [LD] $(TARGET).elf
	@$(CC) -o $(BIN_DIR)/$(TARGET).elf $(LDFLAGS) $(OBJ) $(ASRC:%.s=$(BUILD_DIR)/%.o) $(LDLIBS)
	@echo [HEX] $(TARGET).hex
	@$(OBJCOPY) -O ihex $(BIN_DIR)/$(TARGET).elf $(BIN_DIR)/$(TARGET).hex
	@echo [BIN] $(TARGET).bin
	@$(OBJCOPY) -O binary $(BIN_DIR)/$(TARGET).elf $(BIN_DIR)/$(TARGET).bin

.PHONY: clean

clean:
	@echo [RM] OBJ
	@rm -f $(OBJ)
	@rm -f $(ASRC:%.s=$(BUILD_DIR)/%.o)
	@echo [RM] BIN
	@rm -f $(BIN_DIR)/$(TARGET).elf
	@rm -f $(BIN_DIR)/$(TARGET).hex
	@rm -f $(BIN_DIR)/$(TARGET).bin

flash:
	@st-flash write $(BIN_DIR)/$(TARGET).bin 0x8000000

fpga:
	@echo [FPGA] Building...
	@$(OBJCOPY) --rename-section .data=.rodata,contents,alloc,load,readonly,data -I binary -O elf32-littlearm -B arm7 Resources/FPGA_4.3_snowy_dumped.bin Resources/FPGA_4.3_snowy.o

qemu:
	@sh ./create_image.sh

upload:
	@echo Building and uploading...
	@sh ./buildfw.sh
