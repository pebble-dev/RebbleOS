#pragma once
#include "platform.h"
#include <stddef.h>

void hw_qemu_init(void);
size_t hw_qemu_read(void *buffer, size_t max_len);
size_t hw_qemu_write(const void *buffer, size_t len);
void hw_qemu_irq_enable(void);
