/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2014      */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "../../sdcard/sdcard.h"

static BYTE s_buf[512];

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
	int const sdcard_init_result = sdcard_init();

    if(sdcard_init_result == SD_OK
        || sdcard_init_result == SD_ALREADY_INITIALIZED)
    {
        return 0;
    }
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
	BYTE* buff,		/* Pointer to the destination object */
	DWORD sector,	/* Sector number (LBA) */
	UINT offset,	/* Offset in the sector */
	UINT count		/* Byte count (bit15:destination) */
)
{
    if(buff == 0/*NULL*/)
    {
        return RES_PARERR; // No support for outgoing stream.
    }
    if(offset > 511)
    {
        return RES_PARERR;
    }
    if(count > 512)
    {
        return RES_PARERR;
    }
    if(offset + count > 512)
    {
        return RES_PARERR;
    }

    if(count == 0)
    {
        return RES_OK; // Nothing to do.
    }

    // TODO: Some buffering to speed-up transfers:

    long long const addr = (long long)512 * (long long)sector;

    if(sdcard_blocks_transfer(addr, 1, s_buf, 0) != SD_OK)
    {
        return RES_ERROR;
    }

    for(UINT i = 0;i < count;++i)
    {
        buff[i] = s_buf[offset + i];
    }
    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT disk_writep (
	const BYTE* buff,		/* Pointer to the data to be written, NULL:Initiate/Finalize write operation */
	DWORD sc		/* Sector number (LBA) or Number of bytes to send */
)
{
	DRESULT res = RES_ERROR;


	if (!buff) {
		if (sc) {

			// Initiate write process

		} else {

			// Finalize write process

		}
	} else {

		// Send data to the disk

	}

	return res;
}
