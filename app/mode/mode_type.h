
// Marcel Timm, RhinoDevel, 2020dec20

#ifndef MT_MODE_TYPE
#define MT_MODE_TYPE

enum mode_type
{
    mode_type_err = -1,

    mode_type_save = 0, // Default type supporting all CBM machines via usage of
                        // OS's SAVE and LOAD commands.

    mode_type_pet1 = 1, // Fast loading and saving for CBM/PET BASIC v1.
    mode_type_pet2 = 2, // Fast loading and saving for CBM/PET BASIC v2.
    mode_type_pet4 = 4, // Fast loading and saving for CBM/PET BASIC v4.

    // Fast loading and saving for CBM/PET with wedge at top of memory:
    //
    mode_type_pet1tom = 1 + 0x10, // For BASIC v1.
    mode_type_pet2tom = 2 + 0x10, // For BASIC v2.
    mode_type_pet4tom = 4 + 0x10, // For BASIC v4.
};

#endif //MT_MODE_TYPE
