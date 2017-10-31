#ifndef _SIN_LUT_H
#define _SIN_LUT_H

#include "types.h"

extern const u16 sin_lut[512];

inline s16 lu_sin(u32 theta) { return sin_lut[theta&0x1FF];       }
inline s16 lu_cos(u32 theta) { return sin_lut[(theta+128)&0x1FF]; }

#endif // _SIN_LUT_H
