# Common source.
CFLAGS_all += -IHardware
CFLAGS_all += -IFreeRTOS/include
CFLAGS_all += -IFreeRTOS/portable/GCC/ARM_CM4F
CFLAGS_all += -IPlatform/CMSIS/Include
CFLAGS_all += -ILibraries/UGUI
CFLAGS_all += -ILibraries/neographics/src/
CFLAGS_all += -ILibraries/neographics/src/draw_command
CFLAGS_all += -ILibraries/neographics/src/path
CFLAGS_all += -ILibraries/neographics/src/primitives
CFLAGS_all += -ILibraries/neographics/src/types
CFLAGS_all += -ILibraries/neographics/src/fonts
CFLAGS_all += -ILibraries/neographics/src/text
CFLAGS_all += -IWatchfaces
CFLAGS_all += -IConfig
CFLAGS_all += -IRebbleOS
CFLAGS_all += -IRebbleOS/Gui
CFLAGS_all += -IlibRebbleOS
CFLAGS_all += -IlibRebbleOS/ui
CFLAGS_all += -IlibRebbleOS/ui/layer
CFLAGS_all += -IlibRebbleOS/ui/animation
CFLAGS_all += -IlibRebbleOS/input
CFLAGS_all += -IlibRebbleOS/graphics

CFLAGS_all += -O0 -ggdb -DUSE_STDPERIPH_DRIVER -Wall -ffunction-sections -fdata-sections -mthumb -mlittle-endian -finline-functions -std=gnu99 -falign-functions=16

LDFLAGS_all +=

SRCS_all += FreeRTOS/croutine.c
SRCS_all += FreeRTOS/event_groups.c
SRCS_all += FreeRTOS/list.c
SRCS_all += FreeRTOS/queue.c
SRCS_all += FreeRTOS/tasks.c
SRCS_all += FreeRTOS/timers.c
SRCS_all += FreeRTOS/portable/GCC/ARM_CM4F/port.c
SRCS_all += FreeRTOS/portable/MemMang/heap_4.c

SRCS_all += Libraries/syscall/syscalls.c

SRCS_all += Hardware/stdarg.c

SRCS_all += Libraries/UGUI/ugui.c

SRCS_all += Libraries/neographics/src/common.c
SRCS_all += Libraries/neographics/src/context.c
SRCS_all += Libraries/neographics/src/draw_command/draw_command.c
SRCS_all += Libraries/neographics/src/fonts/fonts.c
SRCS_all += Libraries/neographics/src/path/path.c
SRCS_all += Libraries/neographics/src/primitives/circle.c
SRCS_all += Libraries/neographics/src/primitives/line.c
SRCS_all += Libraries/neographics/src/primitives/rect.c
SRCS_all += Libraries/neographics/src/text/text.c

SRCS_all += RebbleOS/ambient.c
SRCS_all += RebbleOS/appmanager.c
SRCS_all += RebbleOS/backlight.c
SRCS_all += RebbleOS/buttons.c
SRCS_all += RebbleOS/display.c
SRCS_all += RebbleOS/gyro.c
SRCS_all += RebbleOS/main.c
SRCS_all += RebbleOS/power.c
SRCS_all += RebbleOS/rebbleos.c
SRCS_all += RebbleOS/smartstrap.c
SRCS_all += RebbleOS/rebble_time.c
SRCS_all += RebbleOS/rebble_memory.c
SRCS_all += RebbleOS/vibrate.c

SRCS_all += libRebbleOS/librebble.c
SRCS_all += libRebbleOS/math_sin.c
SRCS_all += libRebbleOS/ui/layer/layer.c
SRCS_all += libRebbleOS/ui/layer/scroll_layer.c
SRCS_all += libRebbleOS/ui/layer/text_layer.c
SRCS_all += libRebbleOS/ui/window.c

SRCS_all += Watchfaces/simple.c

SRCS_all += RebbleOS/Gui/gui.c
SRCS_all += RebbleOS/Gui/menu.c
SRCS_all += RebbleOS/Gui/neographics.c

CFLAGS_stm32f4xx = $(CFLAGS_all)
CFLAGS_stm32f4xx += -IPlatform/CMSIS/Device/ST/STM32F4xx/Include
CFLAGS_stm32f4xx += -IPlatform/STM32F4xx_StdPeriph_Driver/inc
CFLAGS_stm32f4xx += -DSTM32F4XX -DSTM32F429_439xx -D__FPU_PRESENT=1 -D__FPU_USED=1 -DARM_MATH_CM4 -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -fsingle-precision-constant

SRCS_stm32f4xx = $(SRCS_all)
SRCS_stm32f4xx += Hardware/startup_stm32f4xx.s
SRCS_stm32f4xx += Hardware/system_stm32f4xx.c
SRCS_stm32f4xx += Hardware/stm32f4x_i2c.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/misc.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_can.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cec.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.c
#SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp.c
#SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_aes.c
#SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_des.c
#SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_tdes.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma2d.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dsi.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash_ramfunc.c
#SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmpi2c.c
#SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
#SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.c
#SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_md5.c
#SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_sha1.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_lptim.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_qspi.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sai.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spdifrx.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c
SRCS_stm32f4xx += Platform/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.c

LDFLAGS_stm32f4xx = $(LDFLAGS_all)
LDFLAGS_stm32f4xx += -Wl,-TUtilities/stm32f4xx.lds

CFLAGS_snowy = $(CFLAGS_stm32f4xx)
CFLAGS_snowy += -IHardware/Pebble_Snowy
CFLAGS_snowy += -DHSI_VALUE=16000000

SRCS_snowy = $(SRCS_stm32f4xx)
SRCS_snowy += Hardware/Pebble_Snowy/snowy_display.c
SRCS_snowy += Hardware/Pebble_Snowy/snowy_backlight.c
SRCS_snowy += Hardware/Pebble_Snowy/snowy_buttons.c
SRCS_snowy += Hardware/Pebble_Snowy/snowy_power.c
SRCS_snowy += Hardware/Pebble_Snowy/snowy_rtc.c
SRCS_snowy += Hardware/Pebble_Snowy/snowy_scanlines.c
SRCS_snowy += Hardware/Pebble_Snowy/snowy_vibrate.c
SRCS_snowy += Hardware/Pebble_Snowy/snowy_ambient.c
SRCS_snowy += Hardware/Pebble_Snowy/snowy.c
SRCS_snowy += Resources/FPGA_4.3_snowy_dumped.bin

LDFLAGS_snowy = $(LDFLAGS_stm32f4xx)

QEMUFLAGS_snowy = -machine pebble-snowy-bb -cpu cortex-m4

PLATFORMS += snowy
