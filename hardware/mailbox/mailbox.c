
// Marcel Timm, RhinoDevel, 2019aug08

#include "mailbox.h"
#include "../../lib/mem/mem.h"
#include "../peribase.h"
//#include "../../lib/assert.h"

#include <stdint.h>

static uint32_t const s_mailbox0_base = PERI_BASE + 0xB880;

static uint32_t const s_mailbox0_read = s_mailbox0_base;
//
// Upper 28 bits hold value, lower 4 bits hold the sending channel's number.

static uint32_t const s_mailbox0_status = s_mailbox0_base + 0x18;

static uint32_t const s_mailbox0_write = s_mailbox0_base + 0x20;
//
// Upper 28 bits hold value, lower 4 bits hold the receiving channel's number.

static uint32_t const s_id_tag_getclockrate = 0x00030002;

static uint32_t const s_status_empty = 0x40000000; // (bit 30 set, if empty)
static uint32_t const s_status_full = 0x80000000; // (bit 31 set, if full)

static uint32_t const s_responsecode_success = 0x80000000;

void mailbox_write(uint32_t const channel, uint32_t const val)
{
    //assert(channel >= 0 && channel < 16);
    //assert((val <= 0x0FFFFFFF);

    while((mem_read(s_mailbox0_status) & s_status_full) != 0)
    {
        // Mailbox is full. Wait.
    }
    mem_write(s_mailbox0_write, (val << 4) | channel);
}

uint32_t mailbox_read(uint32_t const channel)
{
    //assert(channel >= 0 && channel < 16);

    while(true)
    {
        uint32_t val;

        while((mem_read(s_mailbox0_status) & s_status_empty) != 0)
        {
            // Mailbox is empty. Wait.
        }

        val = mem_read(s_mailbox0_read);
        if((val & 0xF) == channel) // Lower 4 bits.
        {
            return val >> 4; // Return higher 28 bits.
        }
    }

    //assert(false);
}

/**
 * - Returns UINT32_MAX on error.
 */
uint32_t mailbox_read_clockrate(enum mailbox_id_clockrate const id)
{
    static uint32_t const channel_nr = 8; // 8 = Property channel nr.

    volatile uint32_t msg_buf[8] __attribute__((aligned (16)));
    //
    // "__attribute__", etc. seems to be GCC-specific..

	uint32_t i = 1;

    // (first array element will be filled, below)
    msg_buf[i++] = 0; // This is a request.

    // Tag starts here:

	msg_buf[i++] = s_id_tag_getclockrate; // Tag identity.
	msg_buf[i++] = 2 * sizeof *msg_buf; // Size of value buffer in byte.
    msg_buf[i++] = 0; // Tag's request code (sending => zero).

    // (uint8_t) value buffer starts here:

    msg_buf[i++] = (uint32_t)id; // Parameter
	msg_buf[i++] = 0; // Used for response, too.

    // (uint8_t) value buffer ended here.

    // Tag ended here.

    msg_buf[i++] = 0; // The end tag.

    msg_buf[0] = sizeof *msg_buf * i; // Total size of buffer in byte (4 * 8).

    // Is it necessary to add a data sync barrier, here [or even some cache
    // cleaning, like "Clean and invalidate DCache entry (MVA)"]?

    mailbox_write(
        channel_nr,
        (0x40000000 + (uint32_t)msg_buf) // TODO: Replace hard-coded conversion to physical memory address. Assuming L2 cache to be enabled, otherwise 0xC0000000 would be correct!
            >> 4);

	mailbox_read(channel_nr); // (return value ignored)

    // Is it necessary to add a data sync barrier, here [or even some cache
    // cleaning, like "Clean and invalidate DCache entry (MVA)"]?

// #ifndef NDEBUG
//     for(i = 0; i < sizeof msg_buf / sizeof *msg_buf; ++i)
//     {
//         console_write("mailbox_read_clockrate: Read at index ");
//         console_write_dword_dec(i);
//         console_write(": 0x");
//         console_write_dword(msg_buf[i]);
//         console_writeline(".");
//     }
// #endif //NDEBUG

	if (msg_buf[1] == s_responsecode_success)
    {
        if(msg_buf[4] == (s_responsecode_success | 8)) // Expecting 8 bytes.
        {
		    return msg_buf[6];
            //
            // (works here, but value buffer is a uint8_t array)
        }
	}
	return UINT32_MAX;
}
