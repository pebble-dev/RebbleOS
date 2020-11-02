/* nrf52_microflash.c
 * flash write routines for internal flash on nRF52840
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "nrfx_nvmc.h"

#include <debug.h>
#include "microflash.h"

#ifdef REBBLEOS
#  error nRF52 Microflash is not supported in softdevice environment
#endif

void hw_microflash_erase(uint32_t addr, uint32_t len) {
    assert((addr & (MICROFLASH_PAGE_SIZE - 1)) == 0);
    assert((len  & (MICROFLASH_PAGE_SIZE - 1)) == 0);
    
    while (len) {
        nrfx_nvmc_page_erase(addr);
        addr += MICROFLASH_PAGE_SIZE;
        len  -= MICROFLASH_PAGE_SIZE;
    }
}

void hw_microflash_write(uint32_t addr, void *data, uint32_t len) {
    assert((addr & 3) == 0);
    assert((len  & 3) == 0);
    
    nrfx_nvmc_words_write(addr, data, len / 4);
    while (!nrfx_nvmc_write_done_check())
        ;
}
