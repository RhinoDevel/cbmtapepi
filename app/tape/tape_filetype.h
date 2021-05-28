
// Marcel Timm, RhinoDevel, 2018jan27

#ifndef MT_TAPE_FILETYPE
#define MT_TAPE_FILETYPE

#ifdef __cplusplus
extern "C" {
#endif

enum tape_filetype
{
    tape_filetype_unknown = 0,
    
    tape_filetype_relocatable = 1, // == BASIC prg.
    //tape_filetype_seq_data_block = 2,
    tape_filetype_non_relocatable = 3, // == Machine language prg.
    //tape_filetype_seq_header = 4,
    //tape_filetype_end_of_tape_marker = 5
};

#ifdef __cplusplus
}
#endif

#endif //MT_TAPE_FILETYPE
