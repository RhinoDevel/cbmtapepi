
// Marcel Timm, RhinoDevel, 2018jan24

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#ifndef MT_PERIBASE
#define MT_PERIBASE

// Choose a single Raspi version to compile for:
//
//#define PERI_BASE_PI_VER 0 // Raspberry Pi Zero.
#define PERI_BASE_PI_VER 1
//#define PERI_BASE_PI_VER 2
//#define PERI_BASE_PI_VER 3
//#define PERI_BASE_PI_VER 4

// Peripherals physical address range start
// (these replace bus address range start 0x7E000000 in bare metal mode,
// see page 6):
//
#define PERI_BASE_PI0AND1 0x20000000 // BCM2835
#define PERI_BASE_PI2AND3 0x3F000000 // BCM2836 and BCM2837.
#define PERI_BASE_PI4AND400 0xFE000000 // BCM2711

#if PERI_BASE_PI_VER == 0
    #define PERI_BASE PERI_BASE_PI0AND1
    #define GPIO_PIN_NR_ACT 47
#elif PERI_BASE_PI_VER == 1
    #define PERI_BASE PERI_BASE_PI0AND1
    #define GPIO_PIN_NR_ACT 16
#elif PERI_BASE_PI_VER == 2
    #define PERI_BASE PERI_BASE_PI2AND3
    #define GPIO_PIN_NR_ACT 47
#elif PERI_BASE_PI_VER == 3
    #define PERI_BASE PERI_BASE_PI2AND3
    //#define GPIO_PIN_NR_ACT // Not available (this way)!
#elif PERI_BASE_PI_VER == 4
    #define PERI_BASE PERI_BASE_PI4AND400 // TODO: Test this in bare metal mode!
    //#define GPIO_PIN_NR_ACT // TODO: Not available (this way)?
#endif //PERI_BASE_PI_VER == 4

#endif //MT_PERIBASE
