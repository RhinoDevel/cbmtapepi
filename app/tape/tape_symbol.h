
// Marcel Timm, RhinoDevel, 2018jan27

#ifndef MT_TAPE_SYMBOL
#define MT_TAPE_SYMBOL

#ifdef __cplusplus
extern "C" {
#endif

// - Each symbol holds two pulses.
//
enum tape_symbol
{
    tape_symbol_zero = 0,
    tape_symbol_one = 1,
    tape_symbol_sync = 2, // Represents TWO sync. pulses!
    tape_symbol_new = 4,
    tape_symbol_end = 8, // (also for transmit block gap start)

    tape_symbol_err = 0xFF // Pseudo-symbol to indicate an error.
};

#ifdef __cplusplus
}
#endif

#endif //MT_TAPE_SYMBOL
