/* nrf52_qspi_flash.c
 * flash routines for QSPI flash on nRF52840
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <debug.h>
#include "rebbleos.h"
#include "nrfx_qspi.h"

/* Asterix has 128Mbit (16MB) of QSPI flash -- a W25Q128JV. */
#define QSPI_JEDEC_ID_W25Q128JV 0xEF4018

/* Asterix-Vla has 64Mbit (8MB) of QSPI flash - a XT25F64B. */
#define QSPI_JEDEC_ID_XT25F64B 0x0B4017

#include "board_config.h"

#define QSPI_INSTR_JEDEC_ID 0x9F

/* We can have out-of-band flash transactions -- like if we break in with
 * GDB and start writing stuff.  */
volatile static int _flash_sync = 0;

static void _flash_handler(nrfx_qspi_evt_t event, void *ctx) {
    assert(event == NRFX_QSPI_EVENT_DONE);
    
    if (_flash_sync) {
        _flash_sync = 0;
        return;
    }
    flash_operation_complete_isr(0);
}

void hw_flash_init() {
    nrfx_qspi_config_t config = NRFX_QSPI_DEFAULT_CONFIG;
    config.phy_if.sck_freq = NRF_QSPI_FREQ_32MDIV1;
    config.pins.sck_pin = BOARD_QSPI_SCK_PIN;
    config.pins.csn_pin = BOARD_QSPI_CSN_PIN;
    config.pins.io0_pin = BOARD_QSPI_IO0_PIN;
    config.pins.io1_pin = BOARD_QSPI_IO1_PIN;
#ifdef BOARD_QSPI_IS_QUAD
    config.pins.io2_pin = BOARD_QSPI_IO2_PIN;
    config.pins.io3_pin = BOARD_QSPI_IO3_PIN;
#else
    config.prot_if.readoc = NRF_QSPI_READOC_FASTREAD;
    config.prot_if.writeoc = NRF_QSPI_WRITEOC_PP;
#endif
    config.irq_priority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;

    nrfx_err_t err;
    err = nrfx_qspi_init(&config, _flash_handler, NULL);
    assert(err == NRFX_SUCCESS && "QSPI initialization failed");

    /* Read the JEDEC ID out of the flash, and give it up to 100ms to come
     * up after the rails stabilize.  */
    int tries = 10;
    do {
        delay_ms(10);
        
        nrf_qspi_cinstr_conf_t instr = NRFX_QSPI_DEFAULT_CINSTR(QSPI_INSTR_JEDEC_ID, 4);
        uint8_t buf[16];
        err = nrfx_qspi_cinstr_xfer(&instr, NULL, buf);
        assert(err == NRFX_SUCCESS && "QSPI JEDEC ID read failed");

        DRV_LOG("flash", APP_LOG_LEVEL_DEBUG, "QSPI: JEDEC ID %02x %02x %02x", buf[0], buf[1], buf[2]);
    
        if ((buf[0] == ((BOARD_QSPI_JEDEC_ID >> 16) & 0xFF)) &&
            (buf[1] == ((BOARD_QSPI_JEDEC_ID >>  8) & 0xFF)) &&
            (buf[2] == ((BOARD_QSPI_JEDEC_ID      ) & 0xFF)))
            break;
    } while (--tries);
    
    assert(tries > 0 && "QSPI JEDEC ID never came back correct");
}

void hw_flash_read_bytes(uint32_t addr, uint8_t *buf, size_t len) {
    nrfx_err_t err;
    
    err = nrfx_qspi_read(buf, len, addr);
    assert(err == NRFX_SUCCESS);
}

void hw_flash_erase_64k_sync(uint32_t addr) {
    nrfx_err_t err;
    
    _flash_sync = 1;
    err = nrfx_qspi_erase(NRF_QSPI_ERASE_LEN_64KB, addr);
    assert(err == NRFX_SUCCESS);

    while (_flash_sync)
        ;
    
    while (nrfx_qspi_mem_busy_check() != NRFX_SUCCESS)
        ;
}

int hw_flash_erase_sync(uint32_t addr, uint32_t len) {
    assert((addr & (64*1024 - 1)) == 0);
    assert((len & (64*1024 - 1)) == 0);
    
    while (len) {
        hw_flash_erase_64k_sync(addr);
        addr += 64*1024;
        len -= 64*1024;
    }
    
    return 0;
}

int hw_flash_write_sync(uint32_t addr, uint8_t *buf, size_t len) {
    nrfx_err_t err;
    
    if (((uint32_t)buf) & 3) {
        DRV_LOG("flash", APP_LOG_LEVEL_ERROR, "HEY!  buf %p was not 4-byte aligned!", buf);
    }

    _flash_sync = 1;
    err = nrfx_qspi_write(buf, len, addr);
    assert(err == NRFX_SUCCESS);

    while (_flash_sync)
        ;
    
    while (nrfx_qspi_mem_busy_check() != NRFX_SUCCESS)
        ;
    
    return 0;
}

__asm__(
"semihosting_syscall:\n"
"       bkpt #0xAB\n"
"       bx lr\n"
);

extern int semihosting_syscall(int c, const void *p);

#define MODE_RB 1
#define MODE_WB 5
static int _open(const char *s, int mode) {
    uint32_t args[3] = {(uint32_t) s, mode, strlen(s)};
    return semihosting_syscall(0x01, args);
}

static int _read(int fd, void *s, int n) {
    uint32_t args[3] = {fd, (uint32_t) s, n};
    return semihosting_syscall(0x06, args);
}

static int _close(int fd) {
    return semihosting_syscall(0x02, (void *)fd);
}

#define BUFLEN 4096

static uint8_t _readbuf[BUFLEN];

void hw_flash_backdoor_load(const char *fname, uint32_t start, uint32_t size) {
    printf("*** Taking over the system in semihosting mode to write data -- hold on tight!\n");
    
    printf("Step 0: opening %s...\n", fname);
    int fd = _open(fname, MODE_RB);
    if (fd < 0) {
        printf("... resource pack open failed\n");
        return;
    }
    printf("... fd %d\n", fd);
    
    uint32_t addr;
    printf("Step 1: erasing region...\n");
    for (addr = start; addr < start + size; addr += 65536) {
        hw_flash_erase_64k_sync(addr);
        printf(".");
    }
    
    printf("Step 2: writing to flash...\n");
    
    int len;
    addr = start;
    while ((len = _read(fd, _readbuf, BUFLEN)) != BUFLEN) {
        len = BUFLEN - len; /* The standard returns the number of bytes *not* filled.  Excuse me? */
        if ((addr & 16383) == 0) printf("...%ld...\n", addr - start);
        hw_flash_write_sync(addr, _readbuf, len);
        addr += len;
    }
    
    printf("...done; wrote %ld bytes\n", addr - start);
    
    _close(fd);
}

void hw_flash_backdoor_load_respack() {
    hw_flash_backdoor_load("build/asterix/res/asterix_res.pbpack", REGION_RES_START, REGION_RES_SIZE);
}

void hw_flash_backdoor_load_fs() {
    hw_flash_backdoor_load("fs.pbfs", REGION_FS_START, REGION_FS_N_PAGES * REGION_FS_PAGE_SIZE);
}
