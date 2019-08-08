
// Marcel Timm, RhinoDevel, 2019aug08

#include "mailbox.h"
#include "../mem/mem.h"
#include "../peribase.h"
//#include "../assert.h"

#include <stdint.h>

static uint32_t const s_mailbox0_base = PERI_BASE + 0xB880;

static uint32_t const s_mailbox0_read = s_mailbox0_base;
//
// Upper 28 bits hold value, lower 4 bits hold the sending channel's number.

static uint32_t const s_mailbox0_status = s_mailbox0_base + 0x18;

static uint32_t const s_mailbox0_write = s_mailbox0_base + 0x20;
//
// Upper 28 bits hold value, lower 4 bits hold the receiving channel's number.

static uint32_t const s_status_empty = 0x40000000; // (bit 30 set, if empty)
static uint32_t const s_status_full = 0x80000000; // (bit 31 set, if full)

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
            return val >> 4; // Higher 28 bits.
        }
    }

    //assert(false);
}
