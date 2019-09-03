// Marcel Timm, RhinoDevel, 2018jan24

#include "busywait.h"
#include "../armtimer/armtimer.h"

void busywait_seconds(uint32_t const seconds)
{
    // Timer counts down 250.000 times in one second (with 250 kHz):
    //
    armtimer_busywait(250000 * seconds, 1000);
}

void busywait_milliseconds(uint32_t const milliseconds)
{
    // Timer counts down 250 times in one millisecond (with 250 kHz):
    //
    armtimer_busywait(250 * milliseconds, 1000);
}

// TODO: Take function time into account (at least it seems so..)!
//
void busywait_microseconds(uint32_t const microseconds)
{
    // Timer counts down 1 time in one microsecond (with 1 MHz):
    //
    armtimer_busywait(microseconds, 250);
}

// TODO: Take function time into account!
//
void busywait_nanoseconds(uint32_t const nanoseconds)
{
    // Timer counts down 1 time in four nanoseconds (with 250 MHz):
    //
    armtimer_busywait(nanoseconds / 4, 1);
}

void busywait_clockcycles(uint32_t const clockcycles)
{
    busywait_nanoseconds(clockcycles * 4); // (1 / 250) MHz == 4ns.
}
