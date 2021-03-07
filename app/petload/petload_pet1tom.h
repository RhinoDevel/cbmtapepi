
// Marcel Timm, RhinoDevel, 2021mar07

#ifndef MT_PETLOAD_PET1TOM
#define MT_PETLOAD_PET1TOM

#include <stdint.h>

// How to get byte array from (PRG) file:
//
// xxd -i pet1tom.prg > pet1tom.h

static uint8_t const s_petload_pet1tom[519] = {
  0x01, 0x04, 0x22, 0x04, 0x9d, 0x1d, 0x9e, 0x31, 0x30, 0x36, 0x30, 0x3a,
  0x8f, 0x20, 0x28, 0x43, 0x29, 0x20, 0x32, 0x30, 0x32, 0x31, 0x2c, 0x20,
  0x52, 0x48, 0x49, 0x4e, 0x4f, 0x44, 0x45, 0x56, 0x45, 0x4c, 0x00, 0x00,
  0x00, 0x38, 0xa5, 0x86, 0xe9, 0x1c, 0x8d, 0xd8, 0x04, 0xa5, 0x87, 0xe9,
  0x01, 0x8d, 0xdc, 0x04, 0x38, 0xa5, 0x86, 0xe9, 0x33, 0x8d, 0x32, 0x05,
  0x8d, 0x44, 0x05, 0x8d, 0x0d, 0x05, 0x8d, 0x1f, 0x05, 0xa5, 0x87, 0xe9,
  0x01, 0x8d, 0x33, 0x05, 0x8d, 0x45, 0x05, 0x8d, 0x0e, 0x05, 0x8d, 0x20,
  0x05, 0x38, 0xa5, 0x86, 0xe9, 0x49, 0x8d, 0x47, 0x05, 0x8d, 0x51, 0x05,
  0x8d, 0x56, 0x05, 0x8d, 0x63, 0x05, 0x8d, 0x68, 0x05, 0x8d, 0x6e, 0x05,
  0xa5, 0x87, 0xe9, 0x00, 0x8d, 0x48, 0x05, 0x8d, 0x52, 0x05, 0x8d, 0x57,
  0x05, 0x8d, 0x64, 0x05, 0x8d, 0x69, 0x05, 0x8d, 0x6f, 0x05, 0x38, 0xa5,
  0x86, 0xe9, 0x23, 0x8d, 0x83, 0x05, 0x8d, 0x88, 0x05, 0x8d, 0x93, 0x05,
  0x8d, 0x98, 0x05, 0x8d, 0x9d, 0x05, 0xa5, 0x87, 0xe9, 0x00, 0x8d, 0x84,
  0x05, 0x8d, 0x89, 0x05, 0x8d, 0x94, 0x05, 0x8d, 0x99, 0x05, 0x8d, 0x9e,
  0x05, 0xa9, 0xd3, 0x85, 0xae, 0xa9, 0x04, 0x85, 0xaf, 0xa9, 0x06, 0x85,
  0xa9, 0xa9, 0x06, 0x85, 0xaa, 0xa5, 0x86, 0x85, 0xa7, 0xa5, 0x87, 0x85,
  0xa8, 0x20, 0xe1, 0xc2, 0x38, 0xa5, 0x86, 0xe9, 0x33, 0x85, 0x86, 0xa5,
  0x87, 0xe9, 0x01, 0x85, 0x87, 0x6c, 0x86, 0x00, 0xa9, 0x4c, 0x85, 0xc2,
  0xa9, 0xea, 0x85, 0xc3, 0xa9, 0x04, 0x85, 0xc4, 0xad, 0x40, 0xe8, 0x29,
  0xf7, 0x8d, 0x40, 0xe8, 0x4c, 0x53, 0xc5, 0xe6, 0xc9, 0xd0, 0x02, 0xe6,
  0xca, 0x84, 0xc5, 0xa4, 0xca, 0xc0, 0x00, 0xd0, 0x2c, 0xa4, 0xc9, 0xc0,
  0x0a, 0xd0, 0x26, 0xa0, 0x00, 0xb1, 0xc9, 0xc9, 0x21, 0xd0, 0x1e, 0xe6,
  0xc9, 0xb1, 0xc9, 0xf0, 0x0c, 0x99, 0xd3, 0x04, 0xc8, 0xc0, 0x10, 0xd0,
  0xf4, 0xb1, 0xc9, 0xd0, 0x0c, 0xa9, 0x20, 0xc0, 0x10, 0xf0, 0x0b, 0x99,
  0xd3, 0x04, 0xc8, 0xd0, 0xf6, 0xa4, 0xc5, 0x4c, 0xc8, 0x00, 0x78, 0xa9,
  0x00, 0x85, 0xc6, 0x85, 0xc7, 0xaa, 0xad, 0xd3, 0x04, 0xc9, 0x2b, 0xd0,
  0x08, 0xa5, 0x7a, 0x85, 0xc6, 0xa5, 0x7b, 0x85, 0xc7, 0x2c, 0x10, 0xe8,
  0xbc, 0xd3, 0x04, 0x20, 0xbd, 0x05, 0xe8, 0xe0, 0x10, 0xd0, 0xf5, 0xa4,
  0xc6, 0x20, 0xbd, 0x05, 0xa4, 0xc7, 0x20, 0xbd, 0x05, 0xa5, 0xc6, 0xd0,
  0x04, 0xa5, 0xc7, 0xf0, 0x22, 0xa4, 0x7c, 0x20, 0xbd, 0x05, 0xa4, 0x7d,
  0x20, 0xbd, 0x05, 0xb1, 0xc6, 0xa8, 0x20, 0xbd, 0x05, 0xe6, 0xc6, 0xd0,
  0x02, 0xe6, 0xc7, 0xa5, 0xc6, 0xc5, 0x7c, 0xd0, 0xee, 0xa5, 0xc7, 0xc5,
  0x7d, 0xd0, 0xe8, 0x20, 0xe3, 0x05, 0x85, 0xc6, 0x20, 0xe3, 0x05, 0x85,
  0xc7, 0xd0, 0x04, 0xa5, 0xc6, 0xf0, 0x21, 0x20, 0xe3, 0x05, 0x85, 0x7c,
  0x20, 0xe3, 0x05, 0x85, 0x7d, 0x20, 0xe3, 0x05, 0x81, 0xc6, 0xe6, 0xc6,
  0xd0, 0x02, 0xe6, 0xc7, 0xa5, 0xc6, 0xc5, 0x7c, 0xd0, 0xef, 0xa5, 0xc7,
  0xc5, 0x7d, 0xd0, 0xe9, 0x58, 0x20, 0x67, 0xc5, 0x20, 0x33, 0xc4, 0x4c,
  0x8b, 0xc3, 0x84, 0xc5, 0xa0, 0x08, 0x46, 0xc5, 0xad, 0x40, 0xe8, 0x29,
  0xf7, 0x90, 0x02, 0x09, 0x08, 0x8d, 0x40, 0xe8, 0xad, 0x13, 0xe8, 0x49,
  0x08, 0x8d, 0x13, 0xe8, 0x2c, 0x11, 0xe8, 0x10, 0xfb, 0x2c, 0x10, 0xe8,
  0x88, 0xd0, 0xdf, 0x60, 0xa2, 0x08, 0x2c, 0x11, 0xe8, 0x10, 0xfb, 0x2c,
  0x10, 0xe8, 0xad, 0x10, 0xe8, 0x29, 0x10, 0x18, 0xf0, 0x01, 0x38, 0x66,
  0xc5, 0xad, 0x40, 0xe8, 0x49, 0x08, 0x8d, 0x40, 0xe8, 0xca, 0xd0, 0xe2,
  0xa5, 0xc5, 0x60
};

#endif //MT_PETLOAD_PET1TOM
