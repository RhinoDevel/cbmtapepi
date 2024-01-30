
// Marcel Timm, RhinoDevel, 2021oct23

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>

#include "mbo.h"
#include "../deb.h"
#include "../inf/inf.h"

#define MSG_BUF_SIZE 12 // Size of message buffer in multiples of 4 bytes.
                        // 48 bytes may not be enough for all responses from
                        // VideoCode (check documentation when necessary).
                        //
                        // Messages must be 16 byte aligned, meaning you might
                        // need padding.


static uint32_t const s_responsecode_success = 0x80000000;

static volatile uint32_t s_msg_buf[MSG_BUF_SIZE] __attribute__((aligned (16)));
//
// "__attribute__", etc. seems to be GCC-specific..

static uint32_t const s_id_tag_allocmem = 0x0003000C;
static uint32_t const s_id_tag_lockmem = 0x0003000D;
static uint32_t const s_id_tag_unlockmem = 0x0003000E;
static uint32_t const s_id_tag_freemem = 0x0003000F;

/*
 * Source:
 * 
 * https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
 * 
 * Message buffer contents for a REQUEST from ARM to VideoCore:
 * -----------------------------------------------------------------------------
 * Index |            | Description
 * -----------------------------------------------------------------------------
 *       |            | - Header starts here -
 *     0 | 0x???????? | Message size in bytes (incl. header, end tag & padding).
 *     1 | 0x00000000 | Always zero for a request.
 *       |            | - Tag starts here -
 *       |            | - Tag header starts here -
 *     2 | 0x???????? | Tag identifier.
 *     3 | 0x???????? | Size of value buffer in bytes.
 *     4 | 0x00000000 | Request code (at least bit 31 must be 0 for a request,
 *       |            |               others are "reserved" - just set to 0).
 *       |            | - Tag data starts here -
 *     5 | May be     | Value buffer (see index 3 for its length in bytes).
 *       | multiple   | May be padding (use zeros) to make the value buffer
 *       | bytes.     | 4 byte aligned.
 *       |            | - End tag starts here -
 *     ? | 0x00000000 | End tag.
 *       |            | - Padding starts here -
 *     ? | ...        | Padding to make full message 16 byte aligned (might not
 *       |            | matter, but fill with zeros).
 * -----------------------------------------------------------------------------
 */

static bool send_buf()
{
    assert(s_msg_buf[0] == sizeof *s_msg_buf * 12);
    assert(s_msg_buf[1] == 0);
    assert(s_msg_buf[4] == 0);

    int fd = open("/dev/vcio", 0);

    if(fd == -1)
    {
        DEB_LOG("Error: Failed to open video core I/O device!");
        return false;
    }

    if(ioctl(fd, _IOWR(100, 0, void*), s_msg_buf) == -1) // Hard-coded
    {
        DEB_LOG("Error: ioctl() failed!");
        close(fd); // Return value ignored!
        return false;
    }

    close(fd); // Return value ignored!
    fd = 0;

    if((s_msg_buf[1] & s_responsecode_success) == 0)
    {
        DEB_LOG("Error: Response signalized error!");
        return false;
    }
    if(s_msg_buf[1] == 0x80000001) // Hard-coded
    {
        DEB_LOG("Error: Response signalized \"partial\"(?) error!");
        return false;
    }

    return true;
}

uint32_t mbo_lock_mem(uint32_t const addr)
{
    if(addr == 0)
    {
        return 0;
    }
    
    s_msg_buf[0] = sizeof *s_msg_buf * 12; // Total size of buffer in bytes.
    s_msg_buf[1] = 0; // This is a request.

    // Tag starts here:

    s_msg_buf[2] = s_id_tag_lockmem; // Tag identity.
    s_msg_buf[3] = 1 * sizeof *s_msg_buf; // Size of value buffer in byte.
    s_msg_buf[4] = 0; // Tag's request code (sending => zero).

    // (uint8_t) value buffer starts here:

    s_msg_buf[5] = addr;

    // (uint8_t) value buffer ended here.

    // Tag ended here.

    s_msg_buf[6] = 0; // The end tag.

    // Padding for 16 byte alignment of full message:
    //
    s_msg_buf[7] = 0;
    s_msg_buf[8] = 0;
    s_msg_buf[9] = 0;
    s_msg_buf[10] = 0;
    s_msg_buf[11] = 0;

    if(!send_buf())
    {
        return 0;
    }
    assert(s_msg_buf[5] > 0);
    return s_msg_buf[5];
}

bool mbo_unlock_mem(uint32_t const addr)
{
    if(addr == 0)
    {
        DEB_LOG("Given address is 0, doing nothing..");
        return true;
    }

    s_msg_buf[0] = sizeof *s_msg_buf * 12; // Total size of buffer in bytes.
    s_msg_buf[1] = 0; // This is a request.

    // Tag starts here:

	s_msg_buf[2] = s_id_tag_unlockmem; // Tag identity.
	s_msg_buf[3] = 1 * sizeof *s_msg_buf; // Size of value buffer in byte.
    s_msg_buf[4] = 0; // Tag's request code (sending => zero).

    // (uint8_t) value buffer starts here:

    s_msg_buf[5] = addr;

    // (uint8_t) value buffer ended here.

    // Tag ended here.

    s_msg_buf[6] = 0; // The end tag.

    // Padding for 16 byte alignment of full message:
    //
    s_msg_buf[7] = 0;
    s_msg_buf[8] = 0;
    s_msg_buf[9] = 0;
    s_msg_buf[10] = 0;
    s_msg_buf[11] = 0;

    if(!send_buf())
    {
        return false;
    }
    assert(s_msg_buf[5] == 0);
    return s_msg_buf[5] == 0;   
}

uint32_t mbo_alloc_mem(
    uint32_t const size, uint32_t const alignment, uint32_t const flags)
{
    if(size % alignment != 0)
    {
        DEB_LOG("Error: Given size is not a multiple of given alignment!");
        return 0;
    }

    s_msg_buf[0] = sizeof *s_msg_buf * 12; // Total size of buffer in bytes.
    s_msg_buf[1] = 0; // This is a request.

    // Tag starts here:

	s_msg_buf[2] = s_id_tag_allocmem; // Tag identity.
	s_msg_buf[3] = 3 * sizeof *s_msg_buf; // Size of value buffer in byte.
    s_msg_buf[4] = 0; // Tag's request code (sending => zero).

    // (uint8_t) value buffer starts here:

    s_msg_buf[5] = size;
	s_msg_buf[6] = alignment;
    s_msg_buf[7] = flags;

    // (uint8_t) value buffer ended here.

    // Tag ended here.

    s_msg_buf[8] = 0; // The end tag.

    // Padding for 16 byte alignment of full message:
    //
    s_msg_buf[9] = 0;
    s_msg_buf[10] = 0;
    s_msg_buf[11] = 0;

    if(!send_buf())
    {
        return 0;
    }
    assert(s_msg_buf[5] != 0);
    return s_msg_buf[5];
}

bool mbo_free_mem(uint32_t const addr)
{
    if(addr == 0)
    {
        DEB_LOG("Given address is 0, doing nothing..");
        return true;
    }

    s_msg_buf[0] = sizeof *s_msg_buf * 12; // Total size of buffer in bytes.
    s_msg_buf[1] = 0; // This is a request.

    // Tag starts here:

	s_msg_buf[2] = s_id_tag_freemem; // Tag identity.
	s_msg_buf[3] = 1 * sizeof *s_msg_buf; // Size of value buffer in byte.
    s_msg_buf[4] = 0; // Tag's request code (sending => zero).

    // (uint8_t) value buffer starts here:

    s_msg_buf[5] = addr;

    // (uint8_t) value buffer ended here.

    // Tag ended here.

    s_msg_buf[6] = 0; // The end tag.

    // Padding for 16 byte alignment of full message:
    //
    s_msg_buf[7] = 0;
    s_msg_buf[8] = 0;
    s_msg_buf[9] = 0;
    s_msg_buf[10] = 0;
    s_msg_buf[11] = 0;

    if(!send_buf())
    {
        return false;
    }
    assert(s_msg_buf[5] == 0);
    return s_msg_buf[5] == 0;
}