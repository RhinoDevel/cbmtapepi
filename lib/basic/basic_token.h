
// Marcel Timm, RhinoDevel, 2019sep15

#ifndef MT_BASIC_TOKEN
#define MT_BASIC_TOKEN

enum basic_token
{
    basic_token_unknown = 0,

    // Commands:

    basic_token_end = 0x80,
    basic_token_for = 0x81,
    basic_token_next = 0x82
    basic_token_data = 0x83
    basic_token_input_ = 0x84, // INPUT#
    basic_token_input = 0x85,
    basic_token_dim = 0x86,
    basic_token_read = 0x87,

    basic_token_let = 0x88,
    basic_token_goto = 0x89,
    basic_token_run = 0x8a,
    basic_token_if = 0x8b,
    basic_token_restore = 0x8c,
    basic_token_gosub = 0x8d,
    basic_token_return = 0x8e,
    basic_token_rem = 0x8f,

    basic_token_stop = 0x90,
    basic_token_on = 0x91,
    basic_token_wait = 0x92,
    basic_token_load = 0x93,
    basic_token_save = 0x94,
    basic_token_verify = 0x95,
    basic_token_def = 0x96,
    basic_token_poke = 0x97,

    basic_token_print_ = 0x98, // PRINT#
    basic_token_print = 0x99,
    basic_token_cont = 0x9a
    basic_token_list = 0x9b,
    basic_token_clr = 0x9c,
    basic_token_cmd = 0x9d,
    basic_token_sys = 0x9e,
    basic_token_open = 0x9f,

    basic_token_close = 0xa0,
    basic_token_get = 0xa1,
    basic_token_new = 0xa2,

    // Miscellaneous "helpers":

    basic_token_tab_ = 0xa3, // TAB(
    basic_token_to = 0xa4,
    basic_token_fn = 0xa5,
    basic_token_spc_ = 0xa6, // SPC(
    basic_token_then = 0xa7,
    basic_token_not = 0xa8,
    basic_token_step = 0xa9,

    // Operators:

    basic_token_add = 0xaa, // +
    basic_token_subtract = 0xab, // -
    basic_token_multiply = 0xac, // *
    basic_token_divide = 0xad, // /
    basic_token_pow = 0xae, // ^
    basic_token_and = 0xaf, // AND
    basic_token_or = 0xb0, // OR
    basic_token_greaterthan = 0xb1, // >	pi
    basic_token_equal = 0xb2, // =
    basic_token_lessthan = 0xb3, // <

    // Functions:

    basic_token_sgn = 0xb4,
    basic_token_int = 0xb5,
    basic_token_abs = 0xb6,
    basic_token_usr = 0xb7,
    basic_token_fre = 0xb8,
    basic_token_pos = 0xb9,
    basic_token_sqr = 0xba,
    basic_token_rnd = 0xbb,
    basic_token_log = 0xbc,
    basic_token_exp = 0xbd,
    basic_token_cos = 0xbe,
    basic_token_sin = 0xbf,
    basic_token_tan = 0xc0,
    basic_token_atn = 0xc1,
    basic_token_peek = 0xc2,
    basic_token_len = 0xc3,
    basic_token_str_ = 0xc4, // STR$
    basic_token_val = 0xc5,
    basic_token_asc = 0xc6,
    basic_token_chr_ = 0xc7, // CHR$
    basic_token_left_ = 0xc8, // LEFT$
    basic_token_right_ = 0xc9, // RIGHT$
    basic_token_mid_ = 0xca, // MID$

    basic_token_go = 0xcb, // Included since v2.0!

    basic_token_pi = 0xff
};

#endif  //MT_BASIC_TOKEN
