#pragma once

#define MICROFLASH_PAGE_SIZE 4096

void hw_microflash_erase(uint32_t addr, uint32_t len);
void hw_microflash_write(uint32_t addr, void *data, uint32_t len);
