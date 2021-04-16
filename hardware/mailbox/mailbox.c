
// Marcel Timm, RhinoDevel, 2019aug08

#include "mailbox.h"
#include "../../lib/mem/mem.h"
#include "../../lib/assert.h"

// #ifndef NDEBUG
//     #include "../../lib/console/console.h"
// #endif //NDEBUG

#include <stdint.h>

#define MSG_BUF_SIZE 12 // Size of message buffer in multiples of 4 bytes.
                        // 48 bytes may not be enough for all responses from
                        // VideoCode (check documentation when necessary).
                        //
                        // Messages must be 16 byte aligned, meaning you might
                        // need padding.

static uint32_t const s_mailbox_base = PERI_BASE + 0xB880;

static uint32_t const s_mailbox0_read = s_mailbox_base;
//
// Upper 28 bits hold value, lower 4 bits hold the sending channel's number.

static uint32_t const s_mailbox0_status = s_mailbox_base + 0x18;

static uint32_t const s_mailbox1_write = s_mailbox_base + 0x20;
//
// Upper 28 bits hold value, lower 4 bits hold the receiving channel's number.

static uint32_t const s_mailbox1_status = s_mailbox_base + 0x38;

// Tag format: 0x000XYZZZ:
//
//             X = Identifies hardware device.
//             Y = Type of command (0 = get, 4 = test, 8 = set).
//             Z = Identifies specific command.

// 0/4:
//
static uint32_t const s_id_tag_getvcfirmwarerev = 0x00000001;
static uint32_t const s_id_tag_getboardmodel    = 0x00010001;
static uint32_t const s_id_tag_getboardrev      = 0x00010002;

// 0/8 (two values of interest in response):
//
static uint32_t const s_id_tag_getarmmemory     = 0x00010005;
static uint32_t const s_id_tag_getvcmemory      = 0x00010006;

// 4/8 (first value in response is ID, second is value of interest):
//
static uint32_t const s_id_tag_getclockrate     = 0x00030002;
static uint32_t const s_id_tag_getmaxclockrate  = 0x00030004;
static uint32_t const s_id_tag_getminclockrate  = 0x00030007;
//
// Temperatures in thousandths of degree celsius:
//
static uint32_t const s_id_tag_getsoctemp       = 0x00030006; // ID = 0.
static uint32_t const s_id_tag_getsocmaxtemp    = 0x0003000a; // ID = 0.

static uint32_t const s_id_tag_setclockrate = 0x00038002;

// Source: https://www.raspberrypi.org/forums/viewtopic.php?f=43&t=109137&start=100#p989907
//
#if PERI_BASE_PI_VER == 3

//static uint32_t const s_id_tag_getgpiostate = 0x00030041;
static uint32_t const s_id_tag_setgpiostate = 0x00038041;

static uint32_t const s_mailbox_id_gpio_actled = 130;

#endif //PERI_BASE_PI_VER == 3

static uint32_t const s_status_empty = 0x40000000; // (bit 30 set, if empty)
static uint32_t const s_status_full = 0x80000000; // (bit 31 set, if full)

static uint32_t const s_responsecode_success = 0x80000000;

static uint32_t const s_channel_nr_propertytags = 8;

static volatile uint32_t s_msg_buf[MSG_BUF_SIZE] __attribute__((aligned (16)));
//
// "__attribute__", etc. seems to be GCC-specific..

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

void mailbox_write(uint32_t const channel, uint32_t const val)
{
    assert(s_msg_buf[0] % 16 == 0);
    assert(channel == 0 || channel < 16);
    assert(val <= 0x0FFFFFFF);

    while((mem_read(s_mailbox1_status) & s_status_full) != 0)
    {
        // Mailbox is full. Wait.
    }
    mem_write(s_mailbox1_write, (val << 4) | channel);
}

uint32_t mailbox_read(uint32_t const channel)
{
    assert(channel == 0 || channel < 16);

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

    assert(false);
}

// #ifndef NDEBUG
//
// static void deb_console_write_msg_buf()
// {
//     for(int i = 0;i < MSG_BUF_SIZE;++i)
//     {
//         console_write_word((uint16_t)i);
//         console_write(": ");
//         console_write_dword(s_msg_buf[i]);
//         console_writeline("");
//     }
// }
//
// #endif //NDEBUG

/**
 * - Returns UINT32_MAX on error or SECOND(!) 32 bit of response value field.
 */
static uint32_t write_and_read(
    uint32_t channel_nr, uint32_t const expected_resp_val_len)
{
// #ifndef NDEBUG
//     console_writeline("write_and_read : Message buffer content before write:");
//     deb_console_write_msg_buf();
// #endif //NDEBUG

    // Is it necessary to add a data sync barrier, here [or even some cache
    // cleaning, like "Clean and invalidate DCache entry (MVA)"]?

    mailbox_write(
        channel_nr,

        // TODO: Replace hard-coded conversion to physical memory address.
        //       Assuming L2 cache to be enabled (for VideoCore),
        //       otherwise 0xC0000000 would be correct!
        //
        //       Also: Is this the correct address for ALL Pis?
        //
        (0x40000000 + (uint32_t)s_msg_buf)
            >> 4);

	mailbox_read(channel_nr); // (return value ignored)

    // Is it necessary to add a data sync barrier, here [or even some cache
    // cleaning, like "Clean and invalidate DCache entry (MVA)"]?

// #ifndef NDEBUG
//     console_writeline("write_and_read : Message buffer content after read:");
//     deb_console_write_msg_buf();
// #endif //NDEBUG

	if (s_msg_buf[1] == s_responsecode_success)
    {
        if(s_msg_buf[4] == (s_responsecode_success | expected_resp_val_len))
        {
            // Hard-coded to return SECOND(!) 32-bit of returned value buffer:
            //
		    return s_msg_buf[6];
            //
            // (works here, but value buffer is a uint8_t array)
        }
	}
	return UINT32_MAX;
}

static uint32_t req0resp4(uint32_t const channel_nr, uint32_t const tag_id)
{
    s_msg_buf[0] = sizeof *s_msg_buf * 8; // Total size of buffer in bytes.
    s_msg_buf[1] = 0; // This is a request.

    // Tag starts here:

	s_msg_buf[2] = tag_id; // Tag identity.
	s_msg_buf[3] = 1 * sizeof *s_msg_buf; // Size of value buffer in byte.
    s_msg_buf[4] = 0; // Tag's request code (sending => zero).

    // (uint8_t) value buffer starts here:

    // (no parameters)
	
    s_msg_buf[5] = 0; // Used for response, only.

    // (uint8_t) value buffer ended here.

    // Tag ended here.

    s_msg_buf[6] = 0; // The end tag.

    s_msg_buf[7] = 0; // Padding for 16 byte alignment.

    if(write_and_read(channel_nr, 4) == UINT32_MAX) // Hard-coded 4.
    {
        return UINT32_MAX; // Error!
    }
    return s_msg_buf[5];
}

static uint32_t req0resp8(
    uint32_t const channel_nr,
    uint32_t const tag_id,
    uint32_t * const out_val_two)
{
    assert(out_val_two != 0);

    s_msg_buf[0] = sizeof *s_msg_buf * 8; // Total size of buffer in bytes.
    s_msg_buf[1] = 0; // This is a request.

    // Tag starts here:

	s_msg_buf[2] = tag_id; // Tag identity.
	s_msg_buf[3] = 2 * sizeof *s_msg_buf; // Size of value buffer in byte.
    s_msg_buf[4] = 0; // Tag's request code (sending => zero).

    // (uint8_t) value buffer starts here:

    // (no parameters)
	
    s_msg_buf[5] = 0; // Used for response, only.
    s_msg_buf[6] = 0; // Used for response, only.

    // (uint8_t) value buffer ended here.

    // Tag ended here.

    s_msg_buf[7] = 0; // The end tag.

    // (no padding necessary, already 16 byte aligned full message)

    if(write_and_read(channel_nr, 8) == UINT32_MAX) // Hard-coded 4.
    {
        *out_val_two = UINT32_MAX;
        return UINT32_MAX; // Error!
    }
    *out_val_two = s_msg_buf[6];
    return s_msg_buf[5];
}

static uint32_t req4resp8(
    uint32_t const channel_nr, uint32_t const tag_id, uint32_t const val_id)
{
    s_msg_buf[0] = sizeof *s_msg_buf * 8; // Total size of buffer in bytes.
    s_msg_buf[1] = 0; // This is a request.

    // Tag starts here:

	s_msg_buf[2] = tag_id; // Tag identity.
	s_msg_buf[3] = 2 * sizeof *s_msg_buf; // Size of value buffer in byte.
    s_msg_buf[4] = 0; // Tag's request code (sending => zero).

    // (uint8_t) value buffer starts here:

    s_msg_buf[5] = (uint32_t)val_id; // Parameter
	s_msg_buf[6] = 0; // Used for response, too.

    // (uint8_t) value buffer ended here.

    // Tag ended here.

    s_msg_buf[7] = 0; // The end tag.

    // (no padding necessary, already 16 byte aligned full message)

    return write_and_read(channel_nr, 8); // Hard-coded 8.
}

uint32_t mailbox_read_vcfirmwarerev()
{
    return req0resp4(s_channel_nr_propertytags, s_id_tag_getvcfirmwarerev);
}
uint32_t mailbox_read_boardmodel()
{
    return req0resp4(s_channel_nr_propertytags, s_id_tag_getboardmodel);
}
uint32_t mailbox_read_boardrev()
{
    return req0resp4(s_channel_nr_propertytags, s_id_tag_getboardrev);
}

uint32_t mailbox_read_armmemory(uint32_t * const out_byte_count)
{
    return req0resp8(
        s_channel_nr_propertytags, s_id_tag_getarmmemory, out_byte_count);
}
uint32_t mailbox_read_vcmemory(uint32_t * const out_byte_count)
{
    return req0resp8(
        s_channel_nr_propertytags, s_id_tag_getvcmemory, out_byte_count);
}

uint32_t mailbox_read_clockrate(enum mailbox_id_clockrate const id)
{
    return req4resp8(
        s_channel_nr_propertytags, s_id_tag_getclockrate, (uint32_t)id);
}
uint32_t mailbox_read_maxclockrate(enum mailbox_id_clockrate const id)
{
    return req4resp8(
        s_channel_nr_propertytags, s_id_tag_getmaxclockrate, (uint32_t)id);
}
uint32_t mailbox_read_minclockrate(enum mailbox_id_clockrate const id)
{
    return req4resp8(
        s_channel_nr_propertytags, s_id_tag_getminclockrate, (uint32_t)id);
}
uint32_t mailbox_read_soctemp()
{
    return req4resp8(s_channel_nr_propertytags, s_id_tag_getsoctemp, 0);
}
uint32_t mailbox_read_socmaxtemp()
{
    return req4resp8(s_channel_nr_propertytags, s_id_tag_getsocmaxtemp, 0);
}

uint32_t mailbox_write_clockrate(
    enum mailbox_id_clockrate const id, uint32_t const val)
{
    s_msg_buf[0] = sizeof *s_msg_buf * 12; // Total size of buffer in bytes.
    s_msg_buf[1] = 0; // This is a request.

    // Tag starts here:

	s_msg_buf[2] = s_id_tag_setclockrate; // Tag identity.
	s_msg_buf[3] = 3 * sizeof *s_msg_buf; // Size of value buffer in byte.
    s_msg_buf[4] = 0; // Tag's request code (sending => zero).

    // (uint8_t) value buffer starts here:

    s_msg_buf[5] = (uint32_t)id; // Parameter
	s_msg_buf[6] = val; // Frequency.
    s_msg_buf[7] = 0; // Skip turbo (for setting ARM frequency above default).

    // (uint8_t) value buffer ended here.

    // Tag ended here.

    s_msg_buf[8] = 0; // The end tag.

    // Padding for 16 byte alignment of full message:
    //
    s_msg_buf[9] = 0;
    s_msg_buf[10] = 0;
    s_msg_buf[11] = 0;

    return write_and_read(s_channel_nr_propertytags, 8); // Hard-coded 8.
}

#if PERI_BASE_PI_VER == 3

/**
 * - Returns UINT32_MAX on error.
 */
uint32_t mailbox_write_gpio_actled(bool const high)
{
    s_msg_buf[0] = sizeof *s_msg_buf * 8; // Total size of buffer in bytes.
    s_msg_buf[1] = 0; // This is a request.

    // Tag starts here:

	s_msg_buf[2] = s_id_tag_setgpiostate; // Tag identity.
	s_msg_buf[3] = 2 * sizeof *s_msg_buf; // Size of value buffer in byte.
    s_msg_buf[4] = 0; // Tag's request code (sending => zero).

    // (uint8_t) value buffer starts here:

    s_msg_buf[5] = s_mailbox_id_gpio_actled; // Parameter
	s_msg_buf[6] = high ? 1 : 0;

    // (uint8_t) value buffer ended here.

    // Tag ended here.

    s_msg_buf[7] = 0; // The end tag.

    // (no padding necessary, already 16 byte aligned full message)

    return write_and_read(s_channel_nr_propertytags, 8); // Hard-coded 8.
}

#endif //PERI_BASE_PI_VER == 3
