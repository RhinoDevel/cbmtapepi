
// Marcel Timm, RhinoDevel, 2018nov16

#ifndef MT_CONFIG
#define MT_CONFIG

#define MT_HEAP_SIZE (32 * 1024 * 1024) // 32 MB.

#define MT_TAPE_GPIO_PIN_NR_READ 2
#define MT_TAPE_GPIO_PIN_NR_SENSE 3
#define MT_TAPE_GPIO_PIN_NR_MOTOR 4
#define MT_TAPE_GPIO_PIN_NR_WRITE 23

#define MT_GPIO_PIN_NR_BUTTON 17
#define MT_GPIO_PIN_NR_LED 27

// Comment this out for non-interactive mode,
// otherwise a menu-based UI will be shown via serial interface
// (it makes a lot of sense to also define NDEBUG, if MT_INTERACTIVE is
// defined):
//
//#define MT_INTERACTIVE

// Set this additionally (for 32 bit):
//
// ./alloc/allocconf.h: #define MT_ALLOC_BITS MT_ALLOC_BITS_32

// Also see ./peribase.h

#endif //MT_CONFIG
