
// Marcel Timm, RhinoDevel, 2019sep04

// *****************************************************************************
// Originally taken from:
//
// https://raw.githubusercontent.com/moizumi99/RPiHaribote/master/haribote/sdcard.h
// *****************************************************************************

#ifndef MT_SDCARD
#define MT_SDCARD

#define SD_OK                  0
#define SD_ERROR               1
#define SD_TIMEOUT             2
#define SD_BUSY                3
#define SD_NOT_INITIALIZED     4
#define SD_ERROR_RESET         5
#define SD_ERROR_CLOCK         6
#define SD_ERROR_VOLTAGE       7
#define SD_ERROR_APP_CMD       8
#define SD_ALREADY_INITIALIZED 9

// This driver is hard-coded for an EMMC frequency of 250 MHz (as to-be-returned
// by mailbox property channel message) and a divisor of 6, resulting in the
// "real" SD card clock speed of 250MHz / 6 = ~41.67 MHz.
//
// Also see: 
//
// https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=94133&p=1219463&hilit=emmc+41#p1220032
//
#define SD_REQ_CLOCKRATE_EMMC ((uint32_t)250000000) // 250MHz

int sdcard_blocks_transfer(
    long long address, int num, unsigned char * buffer, int write);

/** Initialize SD card.
 *
 *  - Returns non-zero value on error.
 *  - Assumes SD card to be present!
 *  - Assumes SD card NEVER getting removed/reinserted!
 *  - Hard-coded for EMMC clock rate of SD_REQ_CLOCKRATE_EMMC MHz (as returned 
 *    by using mailbox property channel to get clock rate).
 */
int sdcard_init();

#endif //MT_SDCARD
