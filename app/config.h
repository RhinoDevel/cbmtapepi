
// Marcel Timm, RhinoDevel, 2018nov16

#ifndef MT_CONFIG
#define MT_CONFIG

#define MT_HEAP_SIZE (32 * 1024 * 1024) // 32 MB.

#define MT_TAPE_GPIO_PIN_NR_READ 10
#define MT_TAPE_GPIO_PIN_NR_SENSE 9
#define MT_TAPE_GPIO_PIN_NR_MOTOR 22
#define MT_TAPE_GPIO_PIN_NR_WRITE 11

#define MT_GPIO_PIN_NR_LED 17

#define MT_FILESYS_ROOT "/"

#define MT_PETSCII_REPLACER 166
//
// Checkerboard

#define MT_ASCII_REPLACER '_'

// Comment this out for non-interactive mode,
// otherwise a menu-based UI will be shown via serial interface
// (it makes a lot of sense to also define NDEBUG, if MT_INTERACTIVE is
// defined):
//
//#define MT_INTERACTIVE

// Set this additionally (for 32 bit):
//
// ./lib/alloc/allocconf.h: #define MT_ALLOC_BITS MT_ALLOC_BITS_32

// Set this additionally (at least for Raspberry Pi 1):
//
// ./lib/alloc/allocconf.h: #define MT_ALLOC_GRANULARITY 4
// ./lib/alloc/allocconf.h: #define MT_ALLOC_ALIGN_TO_GRANULARITY

// Also see: ./hardware/peribase.h

// Also see: ./boot.S

#endif //MT_CONFIG
