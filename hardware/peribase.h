
// Marcel Timm, RhinoDevel, 2018jan24

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#ifndef MT_PERIBASE
#define MT_PERIBASE

// Peripherals physical address range start
// (these replace bus address range start 0x7E000000 in bare metal mode,
// see page 6):
//
#define PERI_BASE_PI1 0x20000000 // BCM2835
#define PERI_BASE_PI2 0x3F000000 // BCM2836

// Choose Raspi type:
//
//#define PERI_BASE PERI_BASE_PI1
#define PERI_BASE PERI_BASE_PI2

#if PERI_BASE == PERI_BASE_PI1
    #define GPIO_PIN_NR_ACT 16
#else //PERI_BASE == PERI_BASE_PI2
    #define GPIO_PIN_NR_ACT 47
#endif //PERI_BASE == PERI_BASE_PI1

#endif //MT_PERIBASE
