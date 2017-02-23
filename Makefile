include config.mk
-include localconfig.mk

# No user serviceable parts below.  You probably want to be editing
# config.mk or localconfig.mk.

GREEN=\033[32;01m
RED=\033[31;01m
STOP=\033[0m

ifeq ($(VERBOSE),)
QUIET = @
SAY = @echo "${GREEN}"$(1)"${STOP}"
else
QUIET =
SAY =
endif

PEBBLE_TOOLCHAIN_PATH ?= /usr/local/Cellar/pebble-toolchain/2.0/arm-cs-tools/bin

PFX ?= $(PEBBLE_TOOLCHAIN_PATH)/arm-none-eabi-

CC = $(PFX)gcc
LD = $(PFX)ld
GDB = $(PFX)gdb
OBJCOPY = $(PFX)objcopy

# output directory
BUILD = build

all: $(PLATFORMS)

# Build rules for each platform, evaluated below.
define PLATFORM_template

$(eval OBJS_$(1) = $(addprefix $(BUILD)/$(1)/,$(addsuffix .o,$(basename $(SRCS_$(1))))) )

$(1): $(BUILD)/$(1)/tintin_fw.bin

$(1)_qemu: $(BUILD)/$(1)/fw.qemu_flash.bin
	qemu-pebble -rtc base=localtime -serial null -serial null -serial stdio -gdb tcp::63770,server $(QEMUFLAGS_$(1)) -pflash $(BUILD)/$(1)/fw.qemu_flash.bin -pflash Resources/$(1)_spi.bin $(QEMUFLAGS)

$(1)_gdb:
	$(PFX)gdb -ex 'target remote localhost:63770' -ex "sym $(BUILD)/$(1)/tintin_fw.elf"

$(BUILD)/$(1)/tintin_fw.elf: $(OBJS_$(1))
	$(call SAY,[$(1)] LD $$@)
	@mkdir -p $$(dir $$@)
	$(QUIET)$(CC) $(CFLAGS_$(1)) $(LDFLAGS_$(1)) -Wl,-Map,$(BUILD)/$(1)/tintin_fw.map -o $$@ $(OBJS_$(1))

# XXX: still need to do dependency tracking here
$(BUILD)/$(1)/%.o: %.c
	$(call SAY,[$(1)] CC $$<)
	@mkdir -p $$(dir $$@)
	$(QUIET)$(CC) $(CFLAGS_$(1)) -c -o $$@ $$< 

$(BUILD)/$(1)/%.o: %.s
	$(call SAY,[$(1)] AS $$<)
	@mkdir -p $$(dir $$@)
	$(QUIET)$(CC) $(CFLAGS_$(1)) -c -o $$@ $$< 

$(BUILD)/$(1)/Resources/FPGA_%.o: Resources/FPGA_%.bin
	$(call SAY,[$(1)] FPGA $$<)
	@mkdir -p $$(dir $$@)
	$(QUIET)$(OBJCOPY) --rename-section .data=.rodata,contents,alloc,load,readonly,data -I binary -O elf32-littlearm -B armv5te $$< $$@

$(BUILD)/$(1)/fw.qemu_flash.bin: Resources/$(1)_boot.bin $(BUILD)/$(1)/tintin_fw.bin
	$(call SAY,[$(1)] QEMU-BIN $$<)
	@mkdir -p $$(dir $$@)
	$(QUIET)cat Resources/$(1)_boot.bin $(BUILD)/$(1)/tintin_fw.bin > $(BUILD)/$(1)/fw.qemu_flash.bin

.PRECIOUS: out/$(1)/tintin_fw.bin out/$(1)/tintin_fw.elf

endef
$(foreach platform,$(PLATFORMS),$(eval $(call PLATFORM_template,$(platform))))

# Build rules that do not depend on target parameters.
%.bin: %.elf
	$(call SAY,OBJCOPY $@)
	$(QUIET)$(PFX)objcopy $< -O binary $@

# XXX: still need to add rules to build a .pbz
