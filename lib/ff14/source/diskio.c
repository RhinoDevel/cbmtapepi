
/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "../../../hardware/sdcard/sdcard.h"

#include <stdbool.h>

static bool s_is_initialized = false;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    (void)pdrv;

    if(!s_is_initialized)
    {
        return STA_NOINIT;
    }
    return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    (void)pdrv;

    if(s_is_initialized)
    {
        return 0;
    }

    int const r = sdcard_init();

    if(r != SD_OK && r != SD_ALREADY_INITIALIZED)
    {
        return STA_NOINIT;
    }
    s_is_initialized = true;
    return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
    (void)pdrv;

	if(!s_is_initialized)
    {
        return RES_NOTRDY;
    }
    if(count == 0)
    {
        return RES_OK; // Nothing to do.
    }
    if(buff == 0/*NULL*/)
    {
        return RES_PARERR;
    }

    long long const addr = 512 * (long long)sector;

    if(sdcard_blocks_transfer(addr, (int)count, (unsigned char *)buff, 0)
        != SD_OK)
    {
        return RES_ERROR;
    }
    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    (void)pdrv;

    if(!s_is_initialized)
    {
        return RES_NOTRDY;
    }
    if(count == 0)
    {
        return RES_OK; // Nothing to do.
    }
    if(buff == 0/*NULL*/)
    {
        return RES_PARERR;
    }

    long long const addr = 512 * (long long)sector;

    if(sdcard_blocks_transfer(addr, (int)count, (unsigned char *)buff, 1)
        != SD_OK)
    {
        return RES_ERROR;
    }
    return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    (void)pdrv;
    (void)buff;

    if(!s_is_initialized)
    {
        return RES_NOTRDY;
    }
    if(cmd != CTRL_SYNC)
    {
        return RES_ERROR;
    }
    return RES_OK;
}
