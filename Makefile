-include localconfig.mk
include config.mk

###########################################################################
#
#    #####          #######         #######         ######            ###
#   #     #            #            #     #         #     #           ###
#   #                  #            #     #         #     #           ###
#    #####             #            #     #         ######             #
#         #            #            #     #         #
#   #     #            #            #     #         #                 ###
#    #####             #            #######         #                 ###
#
#
# You should not have to change anything below unless you are working on
# build infrastructure.  You probably want to be editing config.mk or
# localconfig.mk.
#

GREEN=\033[32;01m
RED=\033[31;01m
STOP=\033[0m

ifeq ($(VERBOSE),)
QUIET = @
SAY = @printf "${GREEN}%s${STOP}\n" "$(1)"
else
QUIET =
SAY =
endif

# Do not override this here!  Override this in localconfig.mk.
ifeq ($(shell uname -s),Darwin)
PEBBLE_TOOLCHAIN_PATH ?= /usr/local/Cellar/pebble-toolchain/2.0/arm-cs-tools/bin
else
PEBBLE_TOOLCHAIN_PATH ?= /usr/bin
endif

# Do not override this here!  Override this in localconfig.mk.
PFX ?= $(PEBBLE_TOOLCHAIN_PATH)/arm-none-eabi-

CC = $(PFX)gcc
LD = $(PFX)ld
GDB = $(PFX)gdb
OBJCOPY = $(PFX)objcopy

# Do not override this here!  Override this in localconfig.mk.
PYTHON3 ?= python3

# Do not override this here!  Override this in localconfig.mk.
QEMU ?= qemu-pebble

# output directory
BUILD = build

all: $(PLATFORMS)

#########################################
# Turn platforms into testable platforms.
#
# This transforms all of the PLATFORMS for which TESTABLE_$(1) is set into
# $(1) and $(1)_test, and then adds that platform to the list of platforms,
# as well as the list of platforms for test.
define PLATFORM_testmaker_template
ifneq ($(TESTABLE_$(1)),)

# Build a new platform.
$(foreach var, \
	CFLAGS SRCS LDFLAGS LIBS QEMUFLAGS QEMUSPITYPE QEMUPACKSIZE QEMUPACKOFS QEMUSPINAME HWREV, \
	$$(eval $(var)_$(1)_test += $($(var)_$(1))) \
	)
$$(eval CFLAGS_$(1)_test += $(CFLAGS_testing))
$$(eval SRCS_$(1)_test += $(SRCS_testing))

# Set this platform as an alias of the other platform.
ifneq ($(PLATFORM_ALIAS_$(1)),)
$$(eval PLATFORM_ALIAS_$(1)_test = $(PLATFORM_ALIAS_$(1)))
else
$$(eval PLATFORM_ALIAS_$(1)_test = $(1))
endif

# Add it to the list of platforms.
PLATFORMS += $(1)_test
PLATFORMS_TESTABLE += $(1)_test

endif
endef
$(foreach platform,$(PLATFORMS),$(eval $(call PLATFORM_testmaker_template,$(platform))))

#################################################
# Build rules for each platform, evaluated below.
#
# This gets very hairy, since some of these are late-bound -- hence the $$s everywhere.
define PLATFORM_template

$$(eval OBJS_$(1) = $(addprefix $(BUILD)/$(1)/,$(addsuffix .o,$(basename $(SRCS_$(1))))) )
$$(eval DEPS_$(1) = $(addprefix $(BUILD)/$(1)/,$(addsuffix .d,$(basename $(SRCS_$(1))))) )

$$(eval CFLAGS_$(1) = $(CFLAGS_$(1)) -I$(BUILD)/$(1)/res )

ifeq ($(PLATFORM_ALIAS_$(1)),)
$$(eval PLATFORM_ALIAS_$(1) = $(1))
endif

-include $$(DEPS_$(1))

$(1): $(BUILD)/$(1)/$(1).pbz

$(1)_qemu: $(BUILD)/$(1)/fw.qemu_flash.bin $(BUILD)/$(1)/fw.qemu_spi.bin
	$(QEMU) -rtc base=localtime -serial null -serial tcp::63771,server,nowait -serial stdio -gdb tcp::63770,server,nowait $(QEMUFLAGS_$(1)) -pflash $(BUILD)/$(1)/fw.qemu_flash.bin -$(QEMUSPITYPE_$(1)) $(BUILD)/$(1)/fw.qemu_spi.bin $(QEMUFLAGS)

$(1)_gdb:
	$(PFX)gdb -ex 'target remote localhost:63770' -ex "sym $(BUILD)/$(1)/tintin_fw.elf"

# List the resource header first to make sure it gets built first ...
# otherwise we could get into trouble.
$(BUILD)/$(1)/tintin_fw.elf: $(BUILD)/$(1)/res/platform_res.h $$(OBJS_$(1))
	$(call SAY,[$(1)] LD $$@)
	@mkdir -p $$(dir $$@)
	$(QUIET)$(CC) $$(CFLAGS_$(1)) $$(LDFLAGS_$(1)) -Wl,-Map,$(BUILD)/$(1)/tintin_fw.map -o $$@ $$(OBJS_$(1)) $$(LIBS_$(1))
	$(QUIET)Utilities/space.sh $(BUILD)/$(1)/tintin_fw.map

$(BUILD)/$(1)/%.o: %.c
	$(call SAY,[$(1)] CC $$<)
	@mkdir -p $$(dir $$@)
	$(QUIET)$(CC) $$(CFLAGS_$(1)) -MMD -MP -MT $$@ -MF $$(addsuffix .d,$$(basename $$@)) -c -o $$@ $$< 

$(BUILD)/$(1)/%.o: %.s
	$(call SAY,[$(1)] AS $$<)
	@mkdir -p $$(dir $$@)
	$(QUIET)$(CC) $$(CFLAGS_$(1)) -c -o $$@ $$< 

$(BUILD)/$(1)/%.o: %.S
	$(call SAY,[$(1)] AS $$<)
	@mkdir -p $$(dir $$@)
	$(QUIET)$(CC) $$(CFLAGS_$(1)) -c -o $$@ $$< 
	
$(BUILD)/$(1)/Resources/%_fpga.o: Resources/%_fpga.bin
	$(call SAY,[$(1)] FPGA $$<)
	@mkdir -p $$(dir $$@)
	$(QUIET)$(OBJCOPY) --rename-section .data=.rodata,contents,alloc,load,readonly,data -I binary -O elf32-littlearm -B armv5te $$< $$@


$(BUILD)/$(1)/fw.qemu_flash.bin: Resources/$$(PLATFORM_ALIAS_$(1))_boot.bin $(BUILD)/$(1)/tintin_fw.bin
	$(call SAY,[$(1)] QEMU-BIN $$@)
	@mkdir -p $$(dir $$@)
	$(QUIET)cat Resources/$$(PLATFORM_ALIAS_$(1))_boot.bin $(BUILD)/$(1)/tintin_fw.bin > $(BUILD)/$(1)/fw.qemu_flash.bin

$(BUILD)/$(1)/$(1).pbz: $(BUILD)/$(1)/tintin_fw.bin $(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res.pbpack LICENSE
	$(call SAY,[$(1)] PBZ $$@)
	$(QUIET)Utilities/mkpbz.py -p $(HWREV_$(1)) -c $(shell git describe --always --dirty --exclude '*') -v $(shell git describe --always --dirty) -l LICENSE $(BUILD)/$(1)/tintin_fw.bin $(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res.pbpack $$@

.PRECIOUS: $(BUILD)/$(1)/tintin_fw.bin $(BUILD)/$(1)/tintin_fw.elf

# Resource build recipe.

-include $(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res.d

# We have to update this first, because otherwise we don't know that we have
# to build the qemu pbpack.
$(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res.d: res/$$(PLATFORM_ALIAS_$(1)).json
	@mkdir -p $$(dir $$@)
	$(QUIET)Utilities/mkpack.py -r res -M $$< $(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res >/dev/null


$(BUILD)/$(1)/res/platform_res.h: $(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res.h
	@rm -f $$@
	@ln -s $$(PLATFORM_ALIAS_$(1))_res.h $$@

$(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res.h: $(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res.pbpack

$(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res.pbpack: res/$$(PLATFORM_ALIAS_$(1)).json
	$(call SAY,[$(1)] MKPACK $$<)
	@mkdir -p $$(dir $$@)
	$(QUIET)Utilities/mkpack.py -r res -M -H -P $$< $(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res

$(BUILD)/$(1)/fw.qemu_spi.bin: Resources/$$(PLATFORM_ALIAS_$(1))_spi.bin $(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res.pbpack
	$(call SAY,[$(1)] QEMU_SPI)
	@mkdir -p $$(dir $$@)
	$(QUIET)cp Resources/$$(PLATFORM_ALIAS_$(1))_spi.bin $$@
	$(QUIET)dd if=/dev/zero of=$$@ bs=1 seek=$(QEMUPACKOFS_$(1)) count=$(QEMUPACKSIZE_$(1)) conv=notrunc || (rm $$@; exit 1)
	$(QUIET)dd if=$(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res.pbpack of=$$@ bs=1 seek=$(QEMUPACKOFS_$(1)) conv=notrunc || (rm $$@; exit 1)

ifneq ($$(RULES_FOR_ALIAS_$$(PLATFORM_ALIAS_$(1))),)
$$(eval RULES_FOR_ALIAS_$$(PLATFORM_ALIAS_$(1)) = true)
Resources/$$(PLATFORM_ALIAS_$(1))_fpga.bin:
	@echo "${RED}Error: platform '$(1)' needs an FPGA binary file in order to build.  Please extract or download one, and put it in $$@. ${STOP}"; exit 1

# workaround for https://savannah.gnu.org/bugs/?15110
res/$$(PLATFORM_ALIAS_$(1)).json:
	@echo "${RED}Error: platform '$$(PLATFORM_ALIAS_$(1))' doesn't have a resource definition file, and some versions of make are buggy and can't tell you this, so I told you myself.  (In particular, the version of GNU Make that Apple ships with is broken, because it's stuck in 2006, because new versions of GNU Make are GPLv3ed, and Apple are huge hosers.)  Your build might fail here, or it might fail later, but it's going to fail -- and now you've learned something new about GNU Make.${STOP}"; exit 1

# Sigh.  This is kind of gross, because it writes outside of the build/
# directory.  On the other hand, the alternative is also pretty gross: it
# would mean that the json file would have to refer to something like
# "../../build/PLATFORM/res/qemu.pbpack", or something.  You gotta take it
# somehow, I suppose.
res/build/pebble-$$(PLATFORM_ALIAS_$(1)).pbpack: res/qemu-tintin-images/$(QEMUSPINAME_$(1))/qemu_spi_flash.bin
	$(call SAY,[$$(PLATFORM_ALIAS_$(1))] PBPACK_EXTRACT) # I dunno, what do *you* think we should call this step?
	@mkdir -p $$(dir $$@)
	$(QUIET)dd if=$$< of=$$@ bs=1 skip=$(QEMUPACKOFS_$(1)) count=$(QEMUPACKSIZE_$(1))

.PRECIOUS: $(BUILD)/$(1)/res/$$(PLATFORM_ALIAS_$(1))_res.pbpack
endif

endef
$(foreach platform,$(PLATFORMS),$(eval $(call PLATFORM_template,$(platform))))

ifeq ($(wildcard res/*),)
$(warning Hmm... res/ seems to be empty.  Did you remember to 'git submodule update --init --recursive'?)
endif

# Build rules that do not depend on target parameters.
%.bin: %.elf
	$(call SAY,OBJCOPY $@)
	$(QUIET)$(PFX)objcopy $< -O binary $@

$(BUILD)/version.c:
	$(call SAY,VERSION $@)
	$(QUIET)mkdir -p $(dir $@)
	$(QUIET)rm -f $@
	$(QUIET)echo "const char git_version[] __attribute__((section(\".version_string.1\"))) = \"$(shell git describe --always --dirty)\";" > $@
	$(QUIET)echo "myx23 mrk1 _ok23o1_oqq[] __k331sl43o__((2om3syx(\".5o12syx_231sxq.c\"))) = \"C4snkny! Lk2 vvkwk2 2yx w48 zovsq1y2k2!\";" | tr '[a-z0-9]' '[0-9a-z]' >> $@
	$(QUIET)echo "const char *const git_authors[] = {" >> $@
	$(QUIET)git shortlog -s | cut -c8- | sort -f | sed -e 's/\(.*\)/    "\1",/' >> $@
	$(QUIET)echo "    0" >> $@
	$(QUIET)echo "};" >> $@

.PHONY: $(BUILD)/version.c

$(BUILD)/testenv: Utilities/requirements.txt
	$(call SAY,RM $@)
	$(QUIET)rm -rf $@
	$(call SAY,VIRTUALENV $@)
	$(QUIET)$(PYTHON3) -m virtualenv $@
	$(call SAY,PIP INSTALL $@)
	$(QUIET)$(BUILD)/testenv/bin/pip3 install -r $<

clean:
	rm -rf $(BUILD)
	rm -rf res/build

.PHONY: clean

# XXX: still need to add rules to build a .pbz
