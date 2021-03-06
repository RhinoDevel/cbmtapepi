
// Marcel Timm, RhinoDevel, 2021feb20

#ifndef MT_PETLOAD_PET2
#define MT_PETLOAD_PET2

#include <stdint.h>

// How to get byte array from (PRG) file:
//
// xxd -i pet2.prg > pet2.h

static uint8_t const s_petload_pet2[305] = {
  0x8f, 0x02, 0xa9, 0x4c, 0x85, 0x70, 0xa9, 0xa6, 0x85, 0x71, 0xa9, 0x02,
  0x85, 0x72, 0xad, 0x40, 0xe8, 0x29, 0xf7, 0x8d, 0x40, 0xe8, 0x4c, 0x5d,
  0xc5, 0xe6, 0x77, 0xd0, 0x02, 0xe6, 0x78, 0x84, 0x73, 0xa4, 0x78, 0xc0,
  0x02, 0xd0, 0x28, 0xa4, 0x77, 0xd0, 0x24, 0xb1, 0x77, 0xc9, 0x21, 0xd0,
  0x1e, 0xe6, 0x77, 0xb1, 0x77, 0xf0, 0x0c, 0x99, 0x8f, 0x02, 0xc8, 0xc0,
  0x10, 0xd0, 0xf4, 0xb1, 0x77, 0xd0, 0x0c, 0xa9, 0x20, 0xc0, 0x10, 0xf0,
  0x0b, 0x99, 0x8f, 0x02, 0xc8, 0xd0, 0xf6, 0xa4, 0x73, 0x4c, 0x76, 0x00,
  0x78, 0xa9, 0x00, 0x85, 0x74, 0x85, 0x75, 0xaa, 0xad, 0x8f, 0x02, 0xc9,
  0x2b, 0xd0, 0x08, 0xa5, 0x28, 0x85, 0x74, 0xa5, 0x29, 0x85, 0x75, 0x2c,
  0x10, 0xe8, 0xbc, 0x8f, 0x02, 0x20, 0x75, 0x03, 0xe8, 0xe0, 0x10, 0xd0,
  0xf5, 0xa4, 0x74, 0x20, 0x75, 0x03, 0xa4, 0x75, 0x20, 0x75, 0x03, 0xa5,
  0x74, 0xd0, 0x04, 0xa5, 0x75, 0xf0, 0x22, 0xa4, 0x2a, 0x20, 0x75, 0x03,
  0xa4, 0x2b, 0x20, 0x75, 0x03, 0xb1, 0x74, 0xa8, 0x20, 0x75, 0x03, 0xe6,
  0x74, 0xd0, 0x02, 0xe6, 0x75, 0xa5, 0x74, 0xc5, 0x2a, 0xd0, 0xee, 0xa5,
  0x75, 0xc5, 0x2b, 0xd0, 0xe8, 0x20, 0x9b, 0x03, 0x85, 0x74, 0x20, 0x9b,
  0x03, 0x85, 0x75, 0xd0, 0x04, 0xa5, 0x74, 0xf0, 0x21, 0x20, 0x9b, 0x03,
  0x85, 0x2a, 0x20, 0x9b, 0x03, 0x85, 0x2b, 0x20, 0x9b, 0x03, 0x81, 0x74,
  0xe6, 0x74, 0xd0, 0x02, 0xe6, 0x75, 0xa5, 0x74, 0xc5, 0x2a, 0xd0, 0xef,
  0xa5, 0x75, 0xc5, 0x2b, 0xd0, 0xe9, 0x58, 0x20, 0x72, 0xc5, 0x20, 0x42,
  0xc4, 0x4c, 0x89, 0xc3, 0x84, 0x73, 0xa0, 0x08, 0x46, 0x73, 0xad, 0x40,
  0xe8, 0x29, 0xf7, 0x90, 0x02, 0x09, 0x08, 0x8d, 0x40, 0xe8, 0xad, 0x13,
  0xe8, 0x49, 0x08, 0x8d, 0x13, 0xe8, 0x2c, 0x11, 0xe8, 0x10, 0xfb, 0x2c,
  0x10, 0xe8, 0x88, 0xd0, 0xdf, 0x60, 0xa2, 0x08, 0x2c, 0x11, 0xe8, 0x10,
  0xfb, 0x2c, 0x10, 0xe8, 0xad, 0x10, 0xe8, 0x29, 0x10, 0x18, 0xf0, 0x01,
  0x38, 0x66, 0x73, 0xad, 0x40, 0xe8, 0x49, 0x08, 0x8d, 0x40, 0xe8, 0xca,
  0xd0, 0xe2, 0xa5, 0x73, 0x60
};

#endif //MT_PETLOAD_PET2