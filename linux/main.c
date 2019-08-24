
// RhinoDevel, Marcel Timm, 2019apr23

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "../hardware/peribase.h"
#include "../lib/mem/mem.h"
#include "../hardware/baregpio/baregpio.h"
#include "../hardware/baregpio/baregpio_params.h"

static void wait_microseconds(uint32_t const microseconds)
{
    usleep(microseconds); // (implicit cast & return value ignored)
}

static void init_gpio()
{
    // Initialize bare GPIO singleton:
    //
    baregpio_init((struct baregpio_params){
        .wait_microseconds = wait_microseconds,

        .mem_read = mem_read,
        .mem_write = mem_write,

        .peri_base = PERI_BASE
    });
}

int main()
{
    init_gpio();

    return 0;
}
