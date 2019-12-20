
// Marcel Timm, RhinoDevel, 2017dec30

// Tested with:
//
// - Commodore/CBM 3001 Series Computer 3032, PET 2001-32N C with Basic 1.0 / ROM v2

// - PET's user port pins interpreted as outputs have LOW level, if PET is powered off.
// - They are "set" to HIGH level (interpreted as outputs) during PET booting up.
// - Initially, the I/O user port pins 0-7 are configured as inputs: PEEK(59459) => 0
// - Such a pin can be configured as output via poking to 59459. E.g. for pin 1: POKE 59459,(PEEK(59459) OR 2) => LOW level (should initially be low..).
// - Output level can be set by poking to 59471. E.g. for pin 1: POKE 59471,(PEEK(59471) OR 2) => HIGH level.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "gpio/gpio.h"
#include "FileSys.h"
#include "Sys.h"
#include "ProgressBar.h"

static const int pin_0_data_to_pet = 4;
static const int pin_1_read_ack_from_pet = 17;
static const int pin_2_wrt_rdy_to_pet = 27;

static const long pet_max_rise_nanoseconds = 2000;
static const long pet_max_fall_nanoseconds = 50.0;

static void sleep_nanoseconds(long const nanoseconds)
{
    struct timespec d;

    clock_gettime(CLOCK_MONOTONIC, &d);

    //long const before = d.tv_nsec,
    //    before_s = d.tv_sec;

    d.tv_nsec += nanoseconds;

    if(d.tv_nsec >= 1000000000L)
    {
        d.tv_nsec -= 1000000000L;
        ++d.tv_sec;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &d, NULL);

    //clock_gettime(CLOCK_MONOTONIC, &d);
    //long const after = d.tv_nsec,
    //    after_s = d.tv_sec;

    //if(before_s==after_s)
    //{
    //    printf("Waited %d nanoseconds.\n", (int)(after-before));
    //}
}

static void setup_pins()
{
    // DATA to PET (init. val. shouldn't matter):
    //
    gpio_set_output(pin_0_data_to_pet, false);

    gpio_set_input_pull_down(pin_1_read_ack_from_pet); // READ ACK from PET.
    gpio_set_output(pin_2_wrt_rdy_to_pet, true); // WRITE READY to PET.
}

static void setup()
{
    assert(gpio_init());
    setup_pins();
}

static void set_output(int const pin_nr, bool const val)
{
    gpio_write(pin_nr, val);
}

static bool get_input(int const pin_nr)
{
    return gpio_read(pin_nr);
}

static void send_bit(unsigned char const bit)
{
    assert(bit==0 || bit==1);

    bool const expected_read_ack = get_input(pin_2_wrt_rdy_to_pet),
        wrt_rdy = !expected_read_ack, // Just toggles.
        val = (bool)bit;
    long const pet_max_nanoseconds = expected_read_ack
        ? pet_max_rise_nanoseconds
        : pet_max_fall_nanoseconds;

    set_output(pin_0_data_to_pet, val); // DATA to PET.
    set_output(pin_2_wrt_rdy_to_pet, wrt_rdy); // WRITE READY to PET.

    while(get_input(pin_1_read_ack_from_pet)!=expected_read_ack)
    {
        sleep_nanoseconds(pet_max_nanoseconds); // TODO: Actually takes ~70us!
    }
}

static void send_byte(unsigned char const byte)
{
    assert(!get_input(pin_1_read_ack_from_pet));
    assert(get_input(pin_2_wrt_rdy_to_pet));

    for(int i = 0;i<8;++i)
    {
        send_bit(byte>>i & 1);
    }
}

int main(int const argc, char * const argv[])
{
    off_t byte_count = 0;
    unsigned char h = 0,
        l = 0;

    if(argc!=2)
    {
        printf(
            "Error: Please call application with PRG source file path as its single argument!\n");
        return EXIT_FAILURE;
    }

    printf("Reading file content..\n");

    unsigned char * const bytes = FileSys_loadFile(argv[1], &byte_count);

    if(bytes==NULL)
    {
        printf("Error: Failed to read file content!\n");
        return EXIT_FAILURE;
    }

    printf("Starting setup..\n");

    setup();

    unsigned int const start_addr = bytes[0]+256*bytes[1],
        payload_len = byte_count-2;

    if(!get_input(pin_1_read_ack_from_pet))
    {
        printf("Error: Input must be set to HIGH!\n");
        free(bytes);
        return EXIT_FAILURE;
    }

    printf("Waiting for READ ACK from PET to fall..\n");

    while(get_input(pin_1_read_ack_from_pet))
    {
        sleep_nanoseconds(pet_max_fall_nanoseconds);
    }

    printf("Starting transfer of 2+2+%d bytes..\n", (int)payload_len);
    h = start_addr/256;
    l = start_addr-256*h;
    printf("Sending start address low byte: %X..\n", l);
    send_byte(l);
    printf("Sending start address high byte: %X..\n", h);
    send_byte(h);

    h = payload_len/256;
    l = payload_len-256*h;
    printf("Sending payload length low byte: %X..\n", l);
    send_byte(l);
    printf("Sending payload length high byte: %X..\n", h);
    send_byte(h);

    uint64_t const t0 = Sys_get_posix_clock_time_ms();

    printf("Sending payload (%d bytes)..\n", (int)payload_len);
    for(int i = 0;i<payload_len;++i)
    {
         send_byte(bytes[i+2]);
         ProgressBar_print(0, i+1, payload_len, 50, true);
    }
    printf("\n");

    printf("Transfer done..\n");

    uint64_t const t_diff = Sys_get_posix_clock_time_ms()-t0;
    double const seconds = (double)t_diff/1000.0;

    printf("Elapsed seconds for payload transfer: %f\n", seconds);
    printf("Bytes per second for payload transfer: %f\n", payload_len/seconds);

    printf("Done.\n");

    return EXIT_SUCCESS;
}
