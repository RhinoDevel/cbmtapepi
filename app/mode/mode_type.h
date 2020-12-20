
// Marcel Timm, RhinoDevel, 2020dec20

#ifndef MT_MODE_TYPE
#define MT_MODE_TYPE

enum mode_type
{
    mode_type_err = -1,
    mode_type_invalid = 0,

    mode_type_save = 1, // Default type supporting all CBM machines via usage of
                        // OS's SAVE and LOAD commands.

    mode_type_pet2 = 2, // Fast loading and saving for CBM/PET BASIC v2.
    mode_type_pet4 = 4 // Fast loading and saving for CBM/PET BASIC v4.
};

#endif //MT_MODE_TYPE
