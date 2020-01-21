
#include "petload.h"
#include "../config.h"
#include "../cbm/cbm_send.h"
#include "../tape/tape_input.h"
#include "../../lib/alloc/alloc.h"
#include "../../lib/basic/basic_addr.h"
#include "../../lib/basic/basic.h"
#include "../../lib/assert.h"
#include "../../hardware/gpio/gpio.h"
#include "../../hardware/armtimer/armtimer.h"
#ifndef NDEBUG
    #include "../../lib/console/console.h"
#endif //NDEBUG

#include <stdint.h>

static char const * const s_name = "petload";

// Retrieve:
//
static uint32_t const s_data_req_to_pet = MT_TAPE_GPIO_PIN_NR_SENSE;
static uint32_t const s_data_ready_from_pet = MT_TAPE_GPIO_PIN_NR_MOTOR;
static uint32_t const s_data_from_pet = MT_TAPE_GPIO_PIN_NR_WRITE;
//
static bool const s_data_ready_from_pet_default_level = false; // LOW
static bool const s_data_req_to_pet_default_level = true; // HIGH

// Send:
//
static uint32_t const s_data_req_from_pet = s_data_from_pet;
static uint32_t const s_data_ready_to_pet = MT_TAPE_GPIO_PIN_NR_READ;
static uint32_t const s_data_to_pet = s_data_req_to_pet;
//
static bool const s_data_ready_to_pet_default_level = true; // HIGH
static bool const s_data_to_pet_default_level = s_data_req_to_pet_default_level;
//
static bool s_send_expected_data_req_level = false;
//
// Set by wait_for_data_req() and petload_send().

// *** BASIC v2 / Rev. 3 ROMs: ***

// static uint16_t const s_addr_key_buf_char_count = 158/*0x009E*/;
//
// Number of characters currently stored in keyboard buffer.

//static uint16_t const s_addr_key_buf = 623/*0x026F*/; // Keyboard buffer.
static uint16_t const s_addr_tape_buf_one = 634/*0x027A*/; // Tape #1 buffer.

// *** ***

static uint16_t const s_addr_offset = 5 + MT_TAPE_INPUT_NAME_LEN; // Magic.

/** Wait for pulse on data-ready line.
 */
static void wait_for_data_ready_pulse()
{
    assert(!s_data_ready_from_pet_default_level);

    // These values depend on motor signal rise and fall times,
    // measured these with attached Raspberry Pi 1 and running CBM Tape Pi
    // transfer at a CBM 3032 as:
    //
    // - Rise time = ~25us.
    // - Fall time = ~50us.
    //
    static uint32_t const pause_rise_microseconds = 2*25; // Measured ~25us.
    static uint32_t const pause_fall_microseconds = 2*50; // Measured ~50us.

    // Default level wait:
    //
    gpio_wait_for_low(s_data_ready_from_pet);
    armtimer_busywait_microseconds(pause_fall_microseconds);

    // Pulse wait:
    //
    gpio_wait_for_high(s_data_ready_from_pet);
    armtimer_busywait_microseconds(pause_rise_microseconds);
    //gpio_wait_for_low(s_data_ready_from_pet);
}

/** Wait for logic level change on data-req. line.
 *
 *  - To be called by send_bit(), only.
 */
static void wait_for_data_req()
{
// #ifndef NDEBUG
//     console_writeline("wait_for_data_req : Waiting for data request..");
// #endif //NDEBUG

    // Measured on user port (!):
    //
    // - PET max. rise time < 2000ns.
    // - PET max. fall time < 50ns.
    //
    static uint32_t const pause_microseconds = 2;

    if(s_send_expected_data_req_level)
    {
        gpio_wait_for_high(s_data_req_from_pet);
    }
    else
    {
        gpio_wait_for_low(s_data_req_from_pet);
    }
    armtimer_busywait_microseconds(pause_microseconds);
    s_send_expected_data_req_level = !s_send_expected_data_req_level;
}

/** Send a data-ready pulse to PET on data-ready line.
 *
 *  - To be called by send_bit(), only.
 */
static void send_data_ready_pulse()
{
    static uint32_t const pulse_microseconds = 5;

    // assert(
    //     gpio_read(s_data_ready_to_pet) == s_data_ready_to_pet_default_level);

    gpio_set_output(s_data_ready_to_pet, !s_data_ready_to_pet_default_level);
    armtimer_busywait_microseconds(pulse_microseconds);
    gpio_set_output(s_data_ready_to_pet, s_data_ready_to_pet_default_level);
}

/**
 * - Bit nrs. given: MSB -> 7 6 5 4 3 2 1 0 <- LSB
 */
static uint8_t retrieve_bit(int const bit_nr)
{
    // Send data-request to Commodore:
    //
    gpio_set_output(
        s_data_req_to_pet,
        bit_nr % 2 == 0
            ? !s_data_req_to_pet_default_level
            : s_data_req_to_pet_default_level);

    wait_for_data_ready_pulse();

    return gpio_read(s_data_from_pet) ? 1 : 0;
}

static void send_bit(uint8_t const bit)
{
    wait_for_data_req();

    gpio_set_output(s_data_to_pet, (bool)bit);

    send_data_ready_pulse();
}

static uint8_t retrieve_byte()
{
    uint8_t ret_val = 0;

    for(int i = 0;i < 8; ++i)
    {
        ret_val |= retrieve_bit(i) << i;
    }
    return ret_val;
}

static void send_byte(uint8_t const byte)
{
    for(int i = 0;i < 8; ++i)
    {
        send_bit(byte >> i & 1);
    }
}

struct tape_input * petload_create()
{
    struct tape_input * const ret_val = alloc_alloc(sizeof *ret_val);
    uint32_t len_buf = 0;
    int i = 0;

    cbm_send_fill_name(ret_val->name, s_name);

    ret_val->type = tape_filetype_relocatable; // Correct for PET.

    ret_val->addr = MT_BASIC_ADDR_PET;
    ret_val->bytes = basic_get_sys(
        ret_val->addr,
        s_addr_tape_buf_one + s_addr_offset,
        true,
        &len_buf);

    assert(len_buf > 0 && len_buf <= 0x0000FFFF);

    ret_val->len = (uint16_t)len_buf;

    ret_val->add_bytes[i++] = 169; // Immediate LDA.
    ret_val->add_bytes[i++] = 83; // Heart symbol (yes, it is romantic).
    ret_val->add_bytes[i++] = 141; // Absolute STA.
    ret_val->add_bytes[i++] = 0; // Lower byte of 32768 (0x8000 - PET video RAM start).
    ret_val->add_bytes[i++] = 128; // Higher byte of 32768.
    ret_val->add_bytes[i++] = 96; // RTS.
    while(i < MT_TAPE_INPUT_ADD_BYTES_LEN)
    {
        ret_val->add_bytes[i++] = 0;
    }

    return ret_val;
}

struct tape_input * petload_retrieve()
{
    assert(gpio_read(s_data_req_to_pet) == s_data_req_to_pet_default_level);

    struct tape_input * ret_val = alloc_alloc(sizeof *ret_val);

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
        console_writeline("petload_retrieve : Retrieved payload bytes.");
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

    assert(gpio_read(s_data_req_to_pet) == s_data_req_to_pet_default_level);

    return ret_val;
}

void petload_send(uint8_t const * const bytes, uint32_t const count)
{
    assert(count >= 2);
    assert((bytes == 0) == (count == 2));

    // Current value is the value of the bit retrieved as last bit via
    // petload_retrieve():
    //
    assert(s_data_from_pet == s_data_req_from_pet);
    s_send_expected_data_req_level = !gpio_read(s_data_req_from_pet);

#ifndef NDEBUG
    console_write("petload_send : Setting data-ready line to ");
    console_write(s_data_ready_to_pet_default_level ? "HIGH" : "LOW");
    console_writeline("..");
#endif //NDEBUG
    gpio_set_output(s_data_ready_to_pet, s_data_ready_to_pet_default_level);

#ifndef NDEBUG
    console_writeline(
        "petload_send : Sending initial start request on data line..");
#endif //NDEBUG
    assert(s_data_req_to_pet == s_data_to_pet);
    assert(s_data_req_to_pet_default_level == s_data_to_pet_default_level);
    assert(
        gpio_read(s_data_to_pet/*s_data_req_to_pet*/)
            == s_data_to_pet_default_level/*s_data_req_to_pet_default_level*/);
    //
    gpio_set_output(
        s_data_to_pet/*s_data_req_to_pet*/,
        !s_data_to_pet_default_level/*s_data_req_to_pet_default_level*/);

#ifndef NDEBUG
    console_write("petload_send : Expecting data-request line to get ");
    console_write(s_send_expected_data_req_level ? "HIGH" : "LOW");
    console_writeline(".");
#endif //NDEBUG

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
#ifndef NDEBUG
    else
    {
#ifndef NDEBUG
        console_write("petload_send : Sending fake address bytes ");
        console_write_byte(0);
        console_write(" and ");
        console_write_byte(0);
        console_writeline("..");
#endif //NDEBUG
        send_byte(0);
        send_byte(0);
        
        console_writeline("petload_send : No limit or payload to send.");
    }
#endif

#ifndef NDEBUG
    console_writeline(
        "petload_send : Setting data line back to default value..");
#endif //NDEBUG
    assert(s_data_req_to_pet == s_data_to_pet);
    assert(s_data_req_to_pet_default_level == s_data_to_pet_default_level);
    gpio_set_output(
        s_data_to_pet/*s_data_req_to_pet*/,
        s_data_to_pet_default_level/*s_data_req_to_pet_default_level*/);

#ifndef NDEBUG
    console_writeline("petload_send : Done.");
#endif //NDEBUG
}

void petload_send_nop()
{
    petload_send(0, 2);
}
