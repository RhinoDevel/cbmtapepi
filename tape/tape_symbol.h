
// Marcel Timm, RhinoDevel, 2018jan27

#ifndef MT_TAPE_SYMBOL
#define MT_TAPE_SYMBOL

// - Each symbol holds two pulses.
//
enum tape_symbol
{
    tape_symbol_zero = 0,
    tape_symbol_one = 1,
    tape_symbol_sync = 2, // Represents TWO sync. pulses!
    tape_symbol_new = 4,
    tape_symbol_end = 8, // (also for transmit block gap start)

    tape_symbol_motor_wait_off = 0xEE, // Pseudo-symbol to disable waiting for
                                       // motor line to get HIGH.
    tape_symbol_done = 0xFF // Pseudo-symbol to stop transfer.
};

#endif //MT_TAPE_SYMBOL
