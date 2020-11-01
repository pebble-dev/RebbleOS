# Common source.
CFLAGS_all += -IHardware
CFLAGS_all += -IFreeRTOS/include
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
CFLAGS_all += -Ircore/protocol
CFLAGS_all += -Ircore/service
CFLAGS_all += -Irwatch
CFLAGS_all += -Irwatch/ui
CFLAGS_all += -Irwatch/ui/notifications
CFLAGS_all += -Irwatch/ui/layer
CFLAGS_all += -Irwatch/ui/animation
CFLAGS_all += -Irwatch/input
CFLAGS_all += -Irwatch/graphics
CFLAGS_all += -Irwatch/event
CFLAGS_all += -Irwatch/storage
CFLAGS_all += -Ihw/platform/qemu
CFLAGS_all += -DNGFX_IS_CORE

# XXX: nostdinc
CFLAGS_all += -O0 -ggdb -Wall -ffunction-sections -fdata-sections -mthumb -mlittle-endian -finline-functions -std=gnu99 -falign-functions=16
# CFLAGS_all += -Wno-implicit-function-declaration
CFLAGS_all += -Wno-unused-variable -Wno-unused-function

LDFLAGS_all += -nostartfiles -nostdlib -Wl,--gc-sections
LIBS_all += -lgcc

# version.c is added as part of the makefile, since it's per platform

CFLAGS_rebbleos += -DREBBLEOS

SRCS_rebbleos += FreeRTOS/croutine.c
SRCS_rebbleos += FreeRTOS/event_groups.c
SRCS_rebbleos += FreeRTOS/list.c
SRCS_rebbleos += FreeRTOS/queue.c
SRCS_rebbleos += FreeRTOS/tasks.c
SRCS_rebbleos += FreeRTOS/timers.c

SRCS_rebbleos += lib/minilib/minilib.c
SRCS_rebbleos += lib/minilib/dprint.c
SRCS_rebbleos += lib/minilib/fmt.c
SRCS_rebbleos += lib/minilib/unfmt.c
SRCS_rebbleos += lib/minilib/rand.c
SRCS_rebbleos += lib/minilib/qalloc.c
SRCS_rebbleos += lib/musl/time/localtime.c
SRCS_rebbleos += lib/musl/time/localtime_r.c
SRCS_rebbleos += lib/musl/time/mktime.c
SRCS_rebbleos += lib/musl/time/__secs_to_tm.c
SRCS_rebbleos += lib/musl/time/__tm_to_secs.c
SRCS_rebbleos += lib/musl/time/__year_to_secs.c
SRCS_rebbleos += lib/musl/time/__month_to_secs.c

SRCS_rebbleos += lib/neographics/src/common.c
SRCS_rebbleos += lib/neographics/src/context.c
SRCS_rebbleos += lib/neographics/src/draw_command/draw_command.c
SRCS_rebbleos += rwatch/graphics/font_file.c
#SRCS_rebbleos += lib/neographics/src/fonts/fonts.c
SRCS_rebbleos += lib/neographics/src/gbitmap/gbitmap.c
SRCS_rebbleos += lib/neographics/src/gbitmap/blit_bw.c
SRCS_rebbleos += lib/neographics/src/gbitmap/blit_color.c
SRCS_rebbleos += lib/neographics/src/path/path.c
SRCS_rebbleos += lib/neographics/src/primitives/circle.c
SRCS_rebbleos += lib/neographics/src/primitives/line.c
SRCS_rebbleos += lib/neographics/src/primitives/rect.c
SRCS_rebbleos += lib/neographics/src/text/text.c
SRCS_rebbleos += lib/neographics/src/types/rect.c

SRCS_rebbleos += lib/png/png.c
SRCS_rebbleos += lib/png/upng.c

SRCS_rebbleos += lib/pbl_strftime/src/strftime.c

SRCS_rebbleos += rcore/ambient.c
SRCS_rebbleos += rcore/appmanager.c
SRCS_rebbleos += rcore/appmanager_app.c
SRCS_rebbleos += rcore/appmanager_app_api.c
SRCS_rebbleos += rcore/appmanager_app_runloop.c
SRCS_rebbleos += rcore/appmanager_app_timer.c
SRCS_rebbleos += rcore/backlight.c
SRCS_rebbleos += rcore/bluetooth.c
SRCS_rebbleos += rcore/ppogatt.c
SRCS_rebbleos += rcore/buttons.c
SRCS_rebbleos += rcore/display.c
SRCS_rebbleos += rcore/debug.c
SRCS_rebbleos += rcore/gyro.c
SRCS_rebbleos += rcore/main.c
SRCS_rebbleos += rcore/notification_manager.c
SRCS_rebbleos += rcore/notification_message.c
SRCS_rebbleos += rcore/power.c
SRCS_rebbleos += rcore/rebbleos.c
SRCS_rebbleos += rcore/smartstrap.c
SRCS_rebbleos += rcore/rebble_time.c
SRCS_rebbleos += rcore/tz.c
SRCS_rebbleos += rcore/rebble_memory.c
SRCS_rebbleos += rcore/vibrate.c
SRCS_rebbleos += rcore/flash.c
SRCS_rebbleos += rcore/fs.c
SRCS_rebbleos += rcore/fs_crc.c
SRCS_rebbleos += rcore/fs_test.c
SRCS_rebbleos += rcore/log.c
SRCS_rebbleos += rcore/resource.c
SRCS_rebbleos += rcore/watchdog.c
SRCS_rebbleos += rcore/overlay_manager.c
SRCS_rebbleos += rcore/rebble_util.c
SRCS_rebbleos += rcore/qemu.c
SRCS_rebbleos += rcore/qemu_endpoints.c
SRCS_rebbleos += rcore/service.c

SRCS_rebbleos += rcore/protocol/protocol_notification.c
SRCS_rebbleos += rcore/protocol/protocol_system.c
SRCS_rebbleos += rcore/protocol/protocol.c
SRCS_rebbleos += rcore/protocol/protocol_app.c
SRCS_rebbleos += rcore/protocol/protocol_blob.c
SRCS_rebbleos += rcore/protocol/protocol_call.c
SRCS_rebbleos += rcore/protocol/protocol_music.c
SRCS_rebbleos += rcore/protocol/protocol_transfer.c

SRCS_rebbleos += rcore/rdb.c
SRCS_rebbleos += rcore/rdb_test.c
SRCS_rebbleos += rcore/prefs.c
SRCS_rebbleos += rcore/service/protocol_service.c
SRCS_rebbleos += rcore/service/timeline.c

SRCS_rebbleos += rwatch/librebble.c
SRCS_rebbleos += rwatch/ngfxwrap.c
SRCS_rebbleos += rwatch/math_sin.c
SRCS_rebbleos += rwatch/ui/layer/layer.c
SRCS_rebbleos += rwatch/ui/layer/bitmap_layer.c
SRCS_rebbleos += rwatch/ui/layer/menu_layer.c
SRCS_rebbleos += rwatch/ui/layer/simple_menu_layer.c
SRCS_rebbleos += rwatch/ui/layer/scroll_layer.c
SRCS_rebbleos += rwatch/ui/layer/action_bar_layer.c
SRCS_rebbleos += rwatch/ui/layer/text_layer.c
SRCS_rebbleos += rwatch/ui/layer/single_notification_layer.c
SRCS_rebbleos += rwatch/ui/layer/inverter_layer.c
SRCS_rebbleos += rwatch/ui/window.c
SRCS_rebbleos += rwatch/ui/notification_window.c
SRCS_rebbleos += rwatch/ui/action_menu.c
SRCS_rebbleos += rwatch/graphics/gbitmap.c
SRCS_rebbleos += rwatch/graphics/graphics.c
SRCS_rebbleos += rwatch/graphics/font_loader.c
SRCS_rebbleos += rwatch/event/tick_timer_service.c
SRCS_rebbleos += rwatch/event/app_timer.c
SRCS_rebbleos += rwatch/event/battery_state_service.c
SRCS_rebbleos += rwatch/event/connection_service.c
SRCS_rebbleos += rwatch/event/event_service.c
SRCS_rebbleos += rwatch/ui/layer/status_bar_layer.c
SRCS_rebbleos += rwatch/ui/animation/animation.c
SRCS_rebbleos += rwatch/ui/animation/property_animation.c
SRCS_rebbleos += rwatch/ui/notifications/battery_overlay.c
SRCS_rebbleos += rwatch/ui/notifications/mini_message.c
SRCS_rebbleos += rwatch/ui/notifications/call_window.c
SRCS_rebbleos += rwatch/ui/notifications/progress_window.c
SRCS_rebbleos += rwatch/ui/vibes.c
SRCS_rebbleos += rwatch/storage/storage_persist.c
SRCS_rebbleos += rwatch/storage/dictionary.c

SRCS_rebbleos += Watchfaces/simple.c
SRCS_rebbleos += Watchfaces/simplicity.c
SRCS_rebbleos += Watchfaces/nivz.c

SRCS_rebbleos += Apps/System/systemapp.c
SRCS_rebbleos += Apps/System/menu.c
SRCS_rebbleos += Apps/System/testapp.c
SRCS_rebbleos += Apps/System/settings.c
SRCS_rebbleos += Apps/System/settings_tz.c

SRCS_rebbleos += Apps/System/widgettest.c
SRCS_rebbleos += Apps/System/notification.c
SRCS_rebbleos += Apps/System/musicapp.c

SRCS_rebbleos += hw/platform/qemu/hw_qemu.c
$(eval $(call platform_include,rebbleos,all))

CFLAGS_testing += -DREBBLEOS_TESTING
SRCS_testing += rcore/test.c

CFLAGS_boot += -DBOOT
SRCS_boot += lib/minilib/minilib.c
SRCS_boot += lib/minilib/fmt.c
SRCS_boot += lib/minilib/dprint.c
SRCS_boot += boot/boot.c
SRCS_boot += boot/gfxbw.c
$(eval $(call platform_include,boot,all))

include hw/chip/stm32f4xx/config.mk
include hw/chip/stm32f2xx/config.mk
include hw/drivers/stm32_dma/config.mk
include hw/drivers/stm32_spi/config.mk
include hw/drivers/stm32_i2c/config.mk
include hw/drivers/stm32_usart/config.mk
include hw/drivers/stm32_buttons/config.mk
include hw/drivers/stm32_power/config.mk
include hw/drivers/stm32_rtc/config.mk
include hw/drivers/stm32_backlight/config.mk
include hw/drivers/stm32_bluetooth_cc256x/config.mk
include hw/platform/snowy_family/config.mk
include hw/platform/snowy/config.mk
include hw/platform/tintin/config.mk
include hw/platform/chalk/config.mk
include Apps/System/tests/config.mk
include lib/btstack/config.mk

ifeq ($(NRF52_SDK_PATH),)
else
include hw/chip/nrf52840/config.mk
include hw/drivers/nrf52_buttons/config.mk
include hw/drivers/nrf52_ls013b7dh05/config.mk
include hw/drivers/nrf52_qspi_flash/config.mk
include hw/drivers/nrf52_bluetooth/config.mk
include hw/platform/asterix/config.mk
endif
