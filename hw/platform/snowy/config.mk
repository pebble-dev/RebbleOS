CFLAGS_snowy = $(CFLAGS_snowy_family)
CFLAGS_snowy += $(CFLAGS_bt)
CFLAGS_snowy += $(CFLAGS_driver_stm32_bluetooth_cc256x)
CFLAGS_snowy += -Ihw/platform/snowy
CFLAGS_snowy += -DHSI_VALUE=16000000 -DREBBLE_PLATFORM=snowy -DREBBLE_PLATFORM_SNOWY

SRCS_snowy = $(SRCS_snowy_family)
SRCS_snowy += $(SRCS_bt)
SRCS_snowy += $(SRCS_driver_stm32_bluetooth_cc256x)
SRCS_snowy += hw/platform/snowy/snowy.c
SRCS_snowy += hw/platform/snowy/snowy_bluetooth.c
SRCS_snowy += Resources/snowy_fpga.bin

SRCS_snowy += build/jerryscript/jerryscript.c
SRCS_snowy += build/jerryscript/jerryscript-libm.c

# Note: rocky_js.c is added to all platforms.
SRCS_snowy += rwatch/js/rocky_canvas.c
SRCS_snowy += rwatch/js/rocky_lib.c
SRCS_snowy += rwatch/js/rocky_setjmp.c
SRCS_snowy += rwatch/js/rocky_port.c

CFLAGS_snowy += -Ilib/jerryscript/jerry-core
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/api
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/include
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/debugger
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/ecma/base
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/ecma/builtin-objects
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/ecma/builtin-objects/typedarray
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/ecma/operations
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/jcontext
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/jmem
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/jrt
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/lit
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/parser/js
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/parser/regexp
CFLAGS_snowy += -Ilib/jerryscript/jerry-core/vm
CFLAGS_snowy += -Ilib/jerryscript/jerry-libm
CFLAGS_snowy += -Ibuild/jerryscript

LDFLAGS_snowy = $(LDFLAGS_snowy_family)
LIBS_snowy = $(LIBS_snowy_family)

QEMUFLAGS_snowy = -machine pebble-snowy-bb -cpu cortex-m4
QEMUSPITYPE_snowy = pflash
QEMUPACKSIZE_snowy = 512000
QEMUPACKOFS_snowy = 3670016
QEMUSPINAME_snowy = basalt/3.0
QEMUSPISIZE_snowy = 16777216
QEMUFSOFS_snowy = 4194304
QEMUFSSIZE_snowy = 12582912

HWREV_snowy = snowy_dvt

TESTABLE_snowy = true

PLATFORMS += snowy
