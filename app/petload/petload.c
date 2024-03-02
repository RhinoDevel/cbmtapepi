
// Marcel Timm, RhinoDevel

#include <stdio.h>

#include "petload_prg_name.h"
#include "petload.h"
#include "petload_pet1.h"
#include "petload_pet1tom.h"
#include "petload_pet2.h"
#include "petload_pet2tom.h"
#include "petload_pet4.h"
#include "petload_pet4tom.h"
#include "petload_vic20tom.h"
#include "petload_c64tof.h"
#include "petload_c64tom.h"
#include "../config.h"
#include "../tape/tape_input.h"
#include "../../lib/alloc/alloc.h"
#include "../../lib/basic/basic_addr.h"
#include "../../lib/basic/basic.h"
#include "../../lib/petasc/petasc.h"
#include "../../lib/assert.h"
#include "../../hardware/gpio/gpio.h"
#ifdef MT_LINUX
    #include <unistd.h> // For usleep().
#else //MT_LINUX
    #include "../../hardware/armtimer/armtimer.h"
#endif //MT_LINUX
#ifndef NDEBUG
    #include "../../lib/console/console.h"
#endif //NDEBUG

#include <stdint.h>
#include <stdbool.h>

// Motor signal rise and fall times,
// measured these with attached Raspberry Pi 1 and running CBM Tape Pi
// transfer at a CBM 3032 as:
//
// - Rise time = ~20us.
// - Fall time = ~40us.
//
static uint32_t const s_motor_rise_microseconds = 20; // Measured ~20us.
static uint32_t const s_motor_fall_microseconds = 40; // Measured ~40us.

// Retrieve:
//
static uint32_t const s_data_from_pet = MT_TAPE_GPIO_PIN_NR_WRITE;
static uint32_t const s_data_ready_from_pet = MT_TAPE_GPIO_PIN_NR_MOTOR;
static uint32_t const s_data_ack_to_pet = MT_TAPE_GPIO_PIN_NR_READ;
//
static bool const s_data_ready_from_pet_default_level = false; // LOW
static bool const s_data_ack_to_pet_default_level = true; // HIGH

// Send:
//
static uint32_t const s_data_to_pet = MT_TAPE_GPIO_PIN_NR_SENSE;
static uint32_t const s_data_ready_to_pet = s_data_ack_to_pet;
static uint32_t const s_data_ack_from_pet = s_data_from_pet;
//
static bool const s_data_ready_to_pet_default_level =
    s_data_ack_to_pet_default_level;
static bool const s_data_to_pet_default_level = true; // HIGH
//
static bool s_send_expected_data_ack_level = false;
//
// Set by retrieve_bit() during retrieval (to get initial value before sending)
// and toggled during send by wait_for_data_ack()

// *** BASIC v2 / Rev. 3 ROMs: ***

// static uint16_t const s_addr_key_buf_char_count = 158/*0x009E*/;
//
// Number of characters currently stored in keyboard buffer.

//static uint16_t const s_addr_key_buf = 623/*0x026F*/; // Keyboard buffer.
//static uint16_t const s_addr_tape_buf_one = 634/*0x027A*/; // Tape #1 buffer.

// *** ***

/** Wait for logic level change on data-ack. line.
 *
 *  - To be called by send_bit(), only.
 */
static void wait_for_data_ack(bool const is_even_bit)
{
//#ifndef NDEBUG
//    console_writeline("wait_for_data_ack : Waiting for ackn. data..");
//#endif //NDEBUG

    // Measured on user port (!):
    //
    // - PET max. rise time < 2000ns.
    // - PET max. fall time < 50ns.
    //
    static uint32_t const pause_rise_microseconds = 4;
    static uint32_t const pause_fall_microseconds = 1;

    gpio_wait_for(
        s_data_ack_from_pet,

        !s_send_expected_data_ack_level,
        //
        // (inverted, because circuit inverts signal from CBM)

        // TODO: Fix this correctly, at least for Linux port (1/2):
        //
#ifndef MT_LINUX
        is_even_bit
            ? 0 // No making-sure necessary for bit 0, 2, 4 and 6.
            :
#endif //MT_LINUX
        (!s_send_expected_data_ack_level
        //
        // (inverted, because circuit inverts signal to CBM)

            ? pause_rise_microseconds
            : pause_fall_microseconds),

        0);

    s_send_expected_data_ack_level = !s_send_expected_data_ack_level;
}

/** Send a data-ack. pulse to PET.
 */
static void send_data_ack_pulse()
{
    static uint32_t const pulse_microseconds = 5;

    //assert(gpio_read(s_data_ack_to_pet) == !s_data_ack_to_pet_default_level);
    //
    // (inverted, because circuit inverts signal to CBM)

    gpio_set_output(s_data_ack_to_pet, s_data_ack_to_pet_default_level);
    //
    // (inverted, because circuit inverts signal to CBM)

#ifdef MT_LINUX
    usleep(pulse_microseconds); // This is NOT precise!
#else //MT_LINUX
    armtimer_busywait_microseconds(pulse_microseconds);
#endif //MT_LINUX
    
    gpio_set_output(s_data_ack_to_pet, !s_data_ack_to_pet_default_level);
    //
    // (inverted, because circuit inverts signal to CBM)
}

/**
 * - Just a wrapper.
 */
static void send_data_ready_pulse()
{
    //assert(s_data_ready_to_pet == s_data_ack_to_pet);

    send_data_ack_pulse();
}

/**
 * - Bit nrs. given: MSB -> 7 6 5 4 3 2 1 0 <- LSB
 */
static uint8_t retrieve_bit(int const bit_nr)
{
    bool const even_bit = bit_nr % 2 == 0,
        wait_for_val = even_bit
            ? !s_data_ready_from_pet_default_level
            : s_data_ready_from_pet_default_level;

//#ifndef NDEBUG
//    console_write("retrieve_bit : Waiting for data-ready to be ");
//    console_write(wait_for_val ? "HIGH" : "LOW");
//    console_writeline("..");
//#endif //NDEBUG

    petload_wait_for_data_ready_val(
        wait_for_val,

        // TODO: Fix this correctly (at least) for Linux port (2/2):
        //
#ifdef MT_LINUX
        true,
#else //MT_LINUX
        even_bit, // Make sure on rise (faster than fall time),
                   // no need to make sure on fall.
#endif //MT_LINUX
        0);

    s_send_expected_data_ack_level = gpio_read(s_data_from_pet);
    //
    // (inverted, because circuit inverts signal from CBM)
    //
    // (because data-from-pet line used here during retrieval
    //  and data-ack.-from-pet line used during send are both using WRITE)


    // Send data-ack to Commodore:
    //
    send_data_ack_pulse();

    return s_send_expected_data_ack_level ? 0 : 1;
}

static void send_bit(uint8_t const bit, bool const is_even_bit)
{
    gpio_set_output(
        s_data_to_pet,
        !(bool)bit); // (inverted, because circuit inverts signal to CBM)

    send_data_ready_pulse();

    wait_for_data_ack(is_even_bit);
}

static uint8_t retrieve_byte()
{
    uint8_t ret_val = 0;

    for(int i = 0;i < 8; ++i)
    {
//#ifndef NDEBUG
//        console_write("retrieve_byte : Retrieving bit nr. ");
//        console_write_byte_dec((uint8_t)i);
//        console_writeline("..");
//#endif //NDEBUG
        ret_val |= retrieve_bit(i) << i;
    }
    return ret_val;
}

static void send_byte(uint8_t const byte)
{
    for(int i = 0;i < 8; ++i)
    {
        send_bit(byte >> i & 1, i % 2 == 0);
    }
}

/** Create tape input object for PRG getting loaded to tape buffer(-s).
 */
static struct tape_input * create(
    uint8_t const * const bytes, int const count)
{
    assert(bytes[0] == 0x8f);
    assert(bytes[1] == 0x02);
    assert(0x028f == 655);
    //                               
    static char const * const name = MT_PETLOAD_PRG_NAME_TAPE_BUF;
    static int const dest_byte_count =
        MT_TAPE_INPUT_ADD_BYTES_LEN
        + 192; // Tape buffer #2 length.

    int const src_byte_count =
        count
        - 2; // For leading PRG start address bytes.

#ifndef NDEBUG
    console_write("petload/create : Needed/source byte count: ");
    console_write_dword_dec((uint32_t)src_byte_count);
    console_writeline(".");

    console_write("petload/create : Available destination byte count: ");
    console_write_dword_dec((uint32_t)dest_byte_count);
    console_writeline(".");
#endif //NDEBUG
    assert(src_byte_count <= dest_byte_count);

    struct tape_input * const ret_val = alloc_alloc(sizeof *ret_val);
    int i = 0,
        src_pos = 2; // Skip leading PRG start address bytes.

    for(i = 0;i < MT_TAPE_INPUT_NAME_LEN;++i)
    {
        ret_val->name[i] = (uint8_t)petasc_get_petscii(
            name[i], MT_PETSCII_REPLACER);
    }
    console_deb_writeline("petload/create : Header name filled.");

    for(i = 0;i < MT_TAPE_INPUT_ADD_BYTES_LEN;++i)
    {
        ret_val->add_bytes[i] = bytes[src_pos];

        ++src_pos;
    }
    console_deb_writeline("petload/create : Header add. bytes filled.");
    assert(src_pos == 2 + MT_TAPE_INPUT_ADD_BYTES_LEN);
    ret_val->addr = (uint16_t)(0x028f + src_pos - 2);

    int const tape_buf_two_used_byte_count = src_byte_count - src_pos + 2;

#ifndef NDEBUG
    console_write("petload/create : Address: 0x");
    console_write_word(ret_val->addr);
    console_writeline(".");

    console_write("petload/create : Used tape buffer #2 byte count: ");
    console_write_dword_dec((uint32_t)tape_buf_two_used_byte_count);
    console_writeline(".");
#endif //NDEBUG

    ret_val->len = (uint16_t)tape_buf_two_used_byte_count;

    ret_val->bytes = alloc_alloc((uint32_t)tape_buf_two_used_byte_count);
    for(i = 0;i < tape_buf_two_used_byte_count;++i)
    {
        ret_val->bytes[i] = bytes[src_pos];

        ++src_pos;
    }
    console_deb_writeline("petload/create : Bytes array filled.");
    assert(src_pos == 2 + src_byte_count);

    ret_val->type = tape_filetype_relocatable; // Correct for PET.

    return ret_val;
}

/** Create tape input object for PRG getting loaded to its start address.
 */
static struct tape_input * create_tom(
    uint8_t const * const bytes, int const count)
{
    static char const * const name = MT_PETLOAD_PRG_NAME_RUN;

    struct tape_input * const ret_val = alloc_alloc(sizeof *ret_val);
    int i = 0;

    for(i = 0;i < MT_TAPE_INPUT_NAME_LEN;++i)
    {
        ret_val->name[i] = (uint8_t)petasc_get_petscii(
            name[i], MT_PETSCII_REPLACER);
    }

    tape_input_fill_add_bytes(ret_val->add_bytes);

    ret_val->addr = (((uint16_t)bytes[1]) << 8) | (uint16_t)bytes[0];
    ret_val->len = (uint16_t)(count - 2);

    ret_val->bytes = alloc_alloc((uint32_t)ret_val->len);
    for(i = 0;i < ret_val->len;++i)
    {
        ret_val->bytes[i] = bytes[i + 2];
    }

    ret_val->type = tape_filetype_relocatable; // Correct for PET.

    return ret_val;
}

void petload_wait_for_data_ready_val(
    bool const wait_for_val,
    bool const do_make_sure,
    bool (*is_stop_requested)())
{
    assert(s_data_ready_from_pet == MT_TAPE_GPIO_PIN_NR_MOTOR);

    static uint32_t const pause_rise_microseconds =
                            2 * s_motor_rise_microseconds;
    static uint32_t const pause_fall_microseconds =
                            2 * s_motor_fall_microseconds;

    gpio_wait_for(
        s_data_ready_from_pet,
        wait_for_val,
        do_make_sure
            ? (wait_for_val
                ? pause_rise_microseconds
                : pause_fall_microseconds)
            : 0,
        is_stop_requested);
}

struct tape_input * petload_create_v1()
{
    return create(
        s_petload_pet1, (int)(sizeof s_petload_pet1 / sizeof *s_petload_pet1));
}
struct tape_input * petload_create_v1tom()
{
    return create_tom(
        s_petload_pet1tom,
        (int)(sizeof s_petload_pet1tom / sizeof *s_petload_pet1tom));
}

struct tape_input * petload_create_v2()
{
    return create(
        s_petload_pet2, (int)(sizeof s_petload_pet2 / sizeof *s_petload_pet2));
}
struct tape_input * petload_create_v2tom()
{
    return create_tom(
        s_petload_pet2tom,
        (int)(sizeof s_petload_pet2tom / sizeof *s_petload_pet2tom));
}

struct tape_input * petload_create_v4()
{
    return create(
        s_petload_pet4, (int)(sizeof s_petload_pet4 / sizeof *s_petload_pet4));
}
struct tape_input * petload_create_v4tom()
{
    return create_tom(
        s_petload_pet4tom,
        (int)(sizeof s_petload_pet4tom / sizeof *s_petload_pet4tom));
}

struct tape_input * petload_create_vic20tom()
{
    return create_tom(
        s_petload_vic20tom,
        (int)(sizeof s_petload_vic20tom / sizeof *s_petload_vic20tom));
}

struct tape_input * petload_create_c64tof()
{
    return create_tom( // (OK, even if not installed in top of BASIC memory)
        s_petload_c64tof,
        (int)(sizeof s_petload_c64tof / sizeof *s_petload_c64tof));
}

struct tape_input * petload_create_c64tom()
{
    return create_tom(
        s_petload_c64tom,
        (int)(sizeof s_petload_c64tom / sizeof *s_petload_c64tom));
}

struct tape_input * petload_retrieve()
{
    // (motor / data-ready from pet line may be low OR on its way to low)

    // TODO: This is a workaround, replace with correct "logic":
    //
    gpio_wait_for(
        s_data_ready_from_pet,
        s_data_ready_from_pet_default_level,
        2 * s_motor_fall_microseconds,
        0);

    struct tape_input * ret_val = alloc_alloc(sizeof *ret_val);

#ifndef NDEBUG
    console_write("petload_retrieve : Setting data-ack. line to ");
    
    console_write(s_data_ack_to_pet_default_level ? "HIGH" : "LOW");
    
    console_writeline(" at CBM..");
#endif //NDEBUG
    gpio_set_output(
        s_data_ack_to_pet,
        !s_data_ack_to_pet_default_level);
        //
        // (inverted, because circuit inverts signal to CBM)

#ifndef NDEBUG
    console_writeline("petload_retrieve : Retrieving \"name\"..");
#endif //NDEBUG

    for(uint32_t i = 0;i < MT_TAPE_INPUT_NAME_LEN;++i)
    {
        ret_val->name[i] = retrieve_byte();
    }
#ifndef NDEBUG
    {
        char * const deb_name = tape_input_create_str_from_name_only(
            ret_val->name);

        console_write("petload_retrieve : Retrieved \"name\" \"");
        console_write(deb_name);
        console_writeline("\".");

        alloc_free(deb_name);
    }
#endif //NDEBUG

    ret_val->addr = (uint16_t)retrieve_byte();
    ret_val->addr |= (uint16_t)retrieve_byte() << 8;
#ifndef NDEBUG
    console_write("petload_retrieve : Retrieved address 0x");
    console_write_word(ret_val->addr);
    console_writeline(".");
#endif //NDEBUG
    if(ret_val->addr != 0)
    {
        uint16_t lim = 0;

        lim = (uint16_t)retrieve_byte();
        lim |= (uint16_t)retrieve_byte() << 8;

        ret_val->len = lim - ret_val->addr;

#ifndef NDEBUG
        console_write("petload_retrieve : Retrieved limit 0x");
        console_write_word(lim);
        console_write(", byte count is 0x");
        console_write_word(ret_val->len);
        console_writeline(".");
#endif //NDEBUG

        ret_val->bytes = alloc_alloc(ret_val->len * sizeof *ret_val->bytes);
        for(uint16_t i = 0;i < ret_val->len; ++i)
        {
            ret_val->bytes[i] = retrieve_byte();
        }
#ifndef NDEBUG
        console_writeline("petload_retrieve : Retrieved payload byte(-s).");
#endif //NDEBUG
    }
    else
    {
        ret_val->len = 0;
        ret_val->bytes = 0;
#ifndef NDEBUG
        console_writeline(
            "petload_retrieve : No limit or payload bytes to retrieve.");
#endif //NDEBUG
    }

    ret_val->type = tape_filetype_relocatable; // Hard-coded

    tape_input_fill_add_bytes(ret_val->add_bytes);

#ifndef NDEBUG
    console_writeline("petload_retrieve : Done.");
#endif //NDEBUG

    assert(gpio_read(s_data_ack_to_pet) != s_data_ack_to_pet_default_level);
    //
    // (inverted, because circuit inverts signal to CBM)
    //
    // (motor / data-ready from pet line may be low OR on its way to low)

    return ret_val;
}

void petload_send(uint8_t const * const bytes, uint32_t const count)
{
    assert(count >= 2);
    assert((bytes == 0) == (count == 2));

    // Expected first data-ack.-from-PET level was already set by retrieval
    // that (necessarily) preceded petload_send() call:
    //
    assert(s_data_ack_from_pet == s_data_from_pet);
    //s_send_expected_data_ack_level
#ifndef NDEBUG
    console_write("petload_send : Expecting first data-ack. value to be ");

    console_write(s_send_expected_data_ack_level ? "HIGH" : "LOW");

    console_writeline(" at CBM.");
#endif //NDEBUG

    // Data-ready-to-PET level is expected to be on its default value,
    // because already set by retrieval that (necessarily) preceded
    // petload_send() call:
    //
    assert(s_data_ready_to_pet == s_data_ack_to_pet);
    assert(s_data_ready_to_pet_default_level
        == s_data_ack_to_pet_default_level);

    assert(gpio_read(s_data_ready_to_pet) != s_data_ready_to_pet_default_level);
    //
    // (inverted, because circuit inverts signal to CBM)

    uint16_t const payload_len = count - 2;

    if(payload_len != 0)
    {
#ifndef NDEBUG
        console_write("petload_send : Sending address bytes ");
        console_write_byte(bytes[0]);
        console_write(" and ");
        console_write_byte(bytes[1]);
        console_writeline("..");
#endif //NDEBUG
        send_byte(bytes[0]);
        send_byte(bytes[1]);

        uint16_t const addr = bytes[1] << 8 | bytes[0],
            lim = addr + payload_len;

#ifndef NDEBUG
        console_write("petload_send : Sending limit bytes ");
        console_write_byte(lim & 0x00FF);
        console_write(" and ");
        console_write_byte(lim >> 8);
        console_writeline("..");
#endif //NDEBUG
        send_byte(lim & 0x00FF);
        send_byte(lim >> 8);

#ifndef NDEBUG
        console_write("petload_send : Sending ");
        console_write_word_dec(payload_len);
        console_writeline(" payload bytes..");
#endif //NDEBUG
        for(uint32_t i = 2;i < count; ++i)
        {
            send_byte(bytes[i]);
        }
    }
    else
    {
        console_deb_writeline("petload_send : No limit or payload to send.");
        
        console_deb_writeline("petload_send : Sending fake address bytes (two zeros)..");
        send_byte(0);
        send_byte(0);
    }

    console_deb_writeline(
        "petload_send : Setting data line back to default value..");
    gpio_set_output(
        s_data_to_pet,
        !s_data_to_pet_default_level);
        //
        // (inverted, because circuit inverts signal to CBM)

    console_deb_writeline("petload_send : Done.");
}

void petload_send_nop()
{
    petload_send(0, 2);
}
