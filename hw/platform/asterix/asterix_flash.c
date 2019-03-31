/* asterix_flash.c
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

#define BOARD_QSPI_SCK_PIN 19
#define BOARD_QSPI_CSN_PIN 17
#define BOARD_QSPI_IO0_PIN 20
#define BOARD_QSPI_IO1_PIN 21
#define BOARD_QSPI_IO2_PIN 22
#define BOARD_QSPI_IO3_PIN 23

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
    config.pins.io2_pin = BOARD_QSPI_IO2_PIN;
    config.pins.io3_pin = BOARD_QSPI_IO3_PIN;
    config.irq_priority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;

    nrfx_err_t err;
    err = nrfx_qspi_init(&config, _flash_handler, NULL);
    assert(err == NRFX_SUCCESS && "QSPI initialization failed");

    /* Read the JEDEC ID out of the flash */    
    nrf_qspi_cinstr_conf_t instr = NRFX_QSPI_DEFAULT_CINSTR(QSPI_INSTR_JEDEC_ID, 4);
    uint8_t buf[16];
    err = nrfx_qspi_cinstr_xfer(&instr, NULL, buf);
    assert(err == NRFX_SUCCESS && "QSPI JEDEC ID read failed");

    DRV_LOG("flash", APP_LOG_LEVEL_DEBUG, "QSPI: JEDEC ID %02x %02x %02x", buf[0], buf[1], buf[2]);
    
    assert((buf[0] == ((QSPI_JEDEC_ID_W25Q128JV >> 16) & 0xFF)) &&
           (buf[1] == ((QSPI_JEDEC_ID_W25Q128JV >>  8) & 0xFF)) &&
           (buf[2] == ((QSPI_JEDEC_ID_W25Q128JV      ) & 0xFF)));
}

void hw_flash_read_bytes(uint32_t addr, uint8_t *buf, size_t len) {
    nrfx_err_t err;

    err = nrfx_qspi_read(buf, len, addr);
    assert(err == NRFX_SUCCESS);
}

void hw_flash_erase_page_sync(uint32_t addr) {
    nrfx_err_t err;
    
    _flash_sync = 1;
    err = nrfx_qspi_erase(NRF_QSPI_ERASE_LEN_4KB, addr);
    assert(err == NRFX_SUCCESS);

    while (_flash_sync)
        ;
    
    while (nrfx_qspi_mem_busy_check() != NRFX_SUCCESS)
        ;
}

void hw_flash_write_sync(uint32_t addr, uint8_t *buf, size_t len) {
    nrfx_err_t err;
    
    _flash_sync = 1;
    err = nrfx_qspi_write(buf, len, addr);
    assert(err == NRFX_SUCCESS);

    while (_flash_sync)
        ;
    
    while (nrfx_qspi_mem_busy_check() != NRFX_SUCCESS)
        ;
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

void hw_flash_write_respack() {
    const char *fname = "build/asterix/res/asterix_res.pbpack";
    
    printf("*** Taking over the system in semihosting mode to write resource pack -- hold on tight!\n");
    
    printf("Step 0: opening resource pack...\n");
    int fd = _open(fname, MODE_RB);
    if (fd < 0) {
        printf("... resource pack open failed\n");
        return;
    }
    printf("... fd %d\n", fd);
    
    uint32_t addr;
    printf("Step 1: erasing system resources region...\n");
    for (addr = REGION_RES_START; addr < REGION_RES_START + REGION_RES_SIZE; addr += 4096)
        hw_flash_erase_page_sync(addr);
    
    printf("Step 2: writing to flash...\n");
    
    int len;
    addr = REGION_RES_START;
    while ((len = _read(fd, _readbuf, BUFLEN)) != BUFLEN) {
        len = BUFLEN - len; /* The standard returns the number of bytes *not* filled.  Excuse me? */
        if ((addr & 16384) == 0) printf("...%ld...\n", addr);
        hw_flash_write_sync(addr, _readbuf, len);
        addr += len;
    }
    
    printf("...done; wrote %ld bytes (last len %d)\n", addr - REGION_RES_START, len);
    
    _close(fd);
}
