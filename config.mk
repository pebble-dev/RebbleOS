# Common source.
CFLAGS_all += -IHardware
CFLAGS_all += -IFreeRTOS/include
CFLAGS_all += -IFreeRTOS/portable/GCC/ARM_CM4F
CFLAGS_all += -IPlatform/CMSIS/Include
CFLAGS_all += -Ilib/neographics/src/
CFLAGS_all += -Ilib/neographics/src/draw_command
CFLAGS_all += -Ilib/neographics/src/path
CFLAGS_all += -Ilib/neographics/src/primitives
CFLAGS_all += -Ilib/neographics/src/types
CFLAGS_all += -Ilib/neographics/src/fonts
CFLAGS_all += -Ilib/neographics/src/text
CFLAGS_all += -Ilib/minilib/inc
CFLAGS_all += -Ilib/png
CFLAGS_all += -Ilib/pbl_strftime/src
CFLAGS_all += -IWatchfaces
CFLAGS_all += -IApps
CFLAGS_all += -IApps/System
CFLAGS_all += -IConfig
CFLAGS_all += -Ircore
CFLAGS_all += -Irwatch
CFLAGS_all += -Irwatch/ui
CFLAGS_all += -Irwatch/ui/layer
CFLAGS_all += -Irwatch/ui/animation
CFLAGS_all += -Irwatch/input
CFLAGS_all += -Irwatch/graphics
CFLAGS_all += -Irwatch/event

# XXX: nostdinc
CFLAGS_all += -O0 -ggdb -Wall -ffunction-sections -fdata-sections -mthumb -mlittle-endian -finline-functions -std=gnu99 -falign-functions=16
# CFLAGS_all += -Wno-implicit-function-declaration
CFLAGS_all += -Wno-unused-variable -Wno-unused-function

LDFLAGS_all += -nostartfiles -nostdlib
LIBS_all += -lgcc

SRCS_all += build/version.c

SRCS_all += FreeRTOS/croutine.c
SRCS_all += FreeRTOS/event_groups.c
SRCS_all += FreeRTOS/list.c
SRCS_all += FreeRTOS/queue.c
SRCS_all += FreeRTOS/tasks.c
SRCS_all += FreeRTOS/timers.c
SRCS_all += FreeRTOS/portable/GCC/ARM_CM4F/port.c
SRCS_all += FreeRTOS/portable/MemMang/heap_4.c

SRCS_all += lib/minilib/minilib.c
SRCS_all += lib/minilib/sbrk.c
SRCS_all += lib/minilib/dprint.c
SRCS_all += lib/minilib/fmt.c
SRCS_all += lib/minilib/unfmt.c
SRCS_all += lib/minilib/rand.c
SRCS_all += lib/musl/time/localtime.c
SRCS_all += lib/musl/time/localtime_r.c
SRCS_all += lib/musl/time/mktime.c
SRCS_all += lib/musl/time/__secs_to_tm.c
SRCS_all += lib/musl/time/__tm_to_secs.c
SRCS_all += lib/musl/time/__year_to_secs.c
SRCS_all += lib/musl/time/__month_to_secs.c

SRCS_all += lib/neographics/src/common.c
SRCS_all += lib/neographics/src/context.c
SRCS_all += lib/neographics/src/draw_command/draw_command.c
SRCS_all += lib/neographics/src/fonts/fonts.c
SRCS_all += lib/neographics/src/path/path.c
SRCS_all += lib/neographics/src/primitives/circle.c
SRCS_all += lib/neographics/src/primitives/line.c
SRCS_all += lib/neographics/src/primitives/rect.c
SRCS_all += lib/neographics/src/text/text.c

SRCS_all += lib/png/png.c
SRCS_all += lib/png/upng.c

SRCS_all += lib/pbl_strftime/src/strftime.c

SRCS_all += rcore/ambient.c
SRCS_all += rcore/appmanager.c
SRCS_all += rcore/backlight.c
SRCS_all += rcore/buttons.c
SRCS_all += rcore/display.c
SRCS_all += rcore/debug.c
SRCS_all += rcore/gyro.c
SRCS_all += rcore/main.c
SRCS_all += rcore/power.c
SRCS_all += rcore/rebbleos.c
SRCS_all += rcore/smartstrap.c
SRCS_all += rcore/rebble_time.c
SRCS_all += rcore/rebble_memory.c
SRCS_all += rcore/vibrate.c
SRCS_all += rcore/flash.c
SRCS_all += rcore/fs.c
SRCS_all += rcore/log.c
SRCS_all += rcore/resource.c
SRCS_all += rcore/heap_app.c
SRCS_all += rcore/watchdog.c

SRCS_all += rwatch/librebble.c
SRCS_all += rwatch/ngfxwrap.c
SRCS_all += rwatch/math_sin.c
SRCS_all += rwatch/ui/layer/layer.c
SRCS_all += rwatch/ui/layer/bitmap_layer.c
SRCS_all += rwatch/ui/layer/menu_layer.c
SRCS_all += rwatch/ui/layer/scroll_layer.c
SRCS_all += rwatch/ui/layer/action_bar_layer.c
SRCS_all += rwatch/ui/layer/text_layer.c
SRCS_all += rwatch/ui/window.c
SRCS_all += rwatch/ui/action_menu.c
SRCS_all += rwatch/ui/notification_window.c
SRCS_all += rwatch/graphics/gbitmap.c
SRCS_all += rwatch/graphics/graphics.c
SRCS_all += rwatch/graphics/font_loader.c
SRCS_all += rwatch/event/tick_timer_service.c
SRCS_all += rwatch/event/app_timer.c
SRCS_all += rwatch/ui/layer/status_bar_layer.c
SRCS_all += rwatch/ui/animation/animation.c
SRCS_all += rwatch/ui/animation/property_animation.c

SRCS_all += Watchfaces/simple.c
SRCS_all += Watchfaces/nivz.c

SRCS_all += Apps/System/systemapp.c
SRCS_all += Apps/System/menu.c

SRCS_all += Apps/System/test.c
SRCS_all += Apps/System/notification.c

include hw/chip/stm32f4xx/config.mk
include hw/chip/stm32f2xx/config.mk
include hw/drivers/stm32_buttons/config.mk
include hw/drivers/stm32_power/config.mk
include hw/platform/snowy_family/config.mk
include hw/platform/snowy/config.mk
include hw/platform/tintin/config.mk
include hw/platform/chalk/config.mk
