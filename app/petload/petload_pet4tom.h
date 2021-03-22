
// Marcel Timm, RhinoDevel, 2021mar07

#ifndef MT_PETLOAD_PET4TOM
#define MT_PETLOAD_PET4TOM

#include <stdint.h>

// How to get byte array from (PRG) file:
//
// xxd -i pet4tom.prg > pet4tom.h

static uint8_t const s_petload_pet4tom[509] = {
  0x01, 0x04, 0x22, 0x04, 0x9d, 0x1d, 0x9e, 0x31, 0x30, 0x36, 0x30, 0x3a,
  0x8f, 0x20, 0x28, 0x43, 0x29, 0x20, 0x32, 0x30, 0x32, 0x31, 0x2c, 0x20,
  0x52, 0x48, 0x49, 0x4e, 0x4f, 0x44, 0x45, 0x56, 0x45, 0x4c, 0x00, 0x00,
  0x00, 0xa6, 0x34, 0xa4, 0x35, 0x38, 0x8a, 0xe9, 0x2c, 0x8d, 0x24, 0x05,
  0x8d, 0x36, 0x05, 0x8d, 0xff, 0x04, 0x8d, 0x11, 0x05, 0x85, 0x34, 0x98,
  0xe9, 0x01, 0x8d, 0x25, 0x05, 0x8d, 0x37, 0x05, 0x8d, 0x00, 0x05, 0x8d,
  0x12, 0x05, 0x85, 0x35, 0x38, 0x8a, 0xe9, 0x1c, 0x8d, 0xc4, 0x04, 0x98,
  0xe9, 0x01, 0x8d, 0xc8, 0x04, 0x38, 0x8a, 0xe9, 0x4d, 0x8d, 0x39, 0x05,
  0x8d, 0x43, 0x05, 0x8d, 0x48, 0x05, 0x8d, 0x55, 0x05, 0x8d, 0x5a, 0x05,
  0x8d, 0x60, 0x05, 0x98, 0xe9, 0x00, 0x8d, 0x3a, 0x05, 0x8d, 0x44, 0x05,
  0x8d, 0x49, 0x05, 0x8d, 0x56, 0x05, 0x8d, 0x5b, 0x05, 0x8d, 0x61, 0x05,
  0x38, 0x8a, 0xe9, 0x25, 0x8d, 0x75, 0x05, 0x8d, 0x7a, 0x05, 0x8d, 0x85,
  0x05, 0x8d, 0x8a, 0x05, 0x8d, 0x8f, 0x05, 0x98, 0xe9, 0x00, 0x8d, 0x76,
  0x05, 0x8d, 0x7b, 0x05, 0x8d, 0x86, 0x05, 0x8d, 0x8b, 0x05, 0x8d, 0x90,
  0x05, 0xa9, 0xe0, 0x85, 0x5c, 0xa9, 0x04, 0x85, 0x5d, 0xa9, 0xfc, 0x85,
  0x57, 0xa9, 0x05, 0x85, 0x58, 0x86, 0x55, 0x84, 0x56, 0x20, 0x57, 0xb3,
  0xa9, 0x4c, 0x85, 0x70, 0xa9, 0xe0, 0x85, 0x71, 0xa9, 0x04, 0x85, 0x72,
  0x78, 0xa0, 0xff, 0xad, 0x40, 0xe8, 0x49, 0x08, 0x8d, 0x40, 0xe8, 0xa2,
  0x2f, 0xca, 0xd0, 0xfd, 0x88, 0xd0, 0xf0, 0x58, 0x60, 0xe6, 0x77, 0xd0,
  0x02, 0xe6, 0x78, 0x84, 0x73, 0xa4, 0x78, 0xc0, 0x02, 0xd0, 0x28, 0xa4,
  0x77, 0xd0, 0x24, 0xb1, 0x77, 0xc9, 0x21, 0xd0, 0x1e, 0xe6, 0x77, 0xb1,
  0x77, 0xf0, 0x0c, 0x99, 0xd0, 0x04, 0xc8, 0xc0, 0x10, 0xd0, 0xf4, 0xb1,
  0x77, 0xd0, 0x0c, 0xa9, 0x20, 0xc0, 0x10, 0xf0, 0x0b, 0x99, 0xd0, 0x04,
  0xc8, 0xd0, 0xf6, 0xa4, 0x73, 0x4c, 0x76, 0x00, 0x78, 0xa9, 0x00, 0x85,
  0x74, 0x85, 0x75, 0xaa, 0xad, 0xd0, 0x04, 0xc9, 0x2b, 0xd0, 0x08, 0xa5,
  0x28, 0x85, 0x74, 0xa5, 0x29, 0x85, 0x75, 0x2c, 0x10, 0xe8, 0xbc, 0xd0,
  0x04, 0x20, 0xaf, 0x05, 0xe8, 0xe0, 0x10, 0xd0, 0xf5, 0xa4, 0x74, 0x20,
  0xaf, 0x05, 0xa4, 0x75, 0x20, 0xaf, 0x05, 0xa5, 0x74, 0xd0, 0x04, 0xa5,
  0x75, 0xf0, 0x22, 0xa4, 0x2a, 0x20, 0xaf, 0x05, 0xa4, 0x2b, 0x20, 0xaf,
  0x05, 0xb1, 0x74, 0xa8, 0x20, 0xaf, 0x05, 0xe6, 0x74, 0xd0, 0x02, 0xe6,
  0x75, 0xa5, 0x74, 0xc5, 0x2a, 0xd0, 0xee, 0xa5, 0x75, 0xc5, 0x2b, 0xd0,
  0xe8, 0x20, 0xd7, 0x05, 0x85, 0x74, 0x20, 0xd7, 0x05, 0x85, 0x75, 0xd0,
  0x04, 0xa5, 0x74, 0xf0, 0x21, 0x20, 0xd7, 0x05, 0x85, 0x2a, 0x20, 0xd7,
  0x05, 0x85, 0x2b, 0x20, 0xd7, 0x05, 0x81, 0x74, 0xe6, 0x74, 0xd0, 0x02,
  0xe6, 0x75, 0xa5, 0x74, 0xc5, 0x2a, 0xd0, 0xef, 0xa5, 0x75, 0xc5, 0x2b,
  0xd0, 0xe9, 0x58, 0x20, 0xe9, 0xb5, 0x20, 0xb6, 0xb4, 0x4c, 0xff, 0xb3,
  0x84, 0x73, 0xa0, 0x08, 0x46, 0x73, 0xad, 0x40, 0xe8, 0x29, 0xf7, 0x90,
  0x02, 0x09, 0x08, 0x8d, 0x40, 0xe8, 0xad, 0x13, 0xe8, 0x49, 0x08, 0x8d,
  0x13, 0xe8, 0xad, 0x11, 0xe8, 0x29, 0x80, 0xf0, 0xf9, 0x2c, 0x10, 0xe8,
  0x88, 0xd0, 0xdd, 0x60, 0xa2, 0x08, 0xad, 0x11, 0xe8, 0x29, 0x80, 0xf0,
  0xf9, 0x2c, 0x10, 0xe8, 0xad, 0x10, 0xe8, 0x29, 0x10, 0x18, 0xf0, 0x01,
  0x38, 0x66, 0x73, 0xad, 0x40, 0xe8, 0x49, 0x08, 0x8d, 0x40, 0xe8, 0xca,
  0xd0, 0xe0, 0xa5, 0x73, 0x60
};

#endif //MT_PETLOAD_PET4TOM
