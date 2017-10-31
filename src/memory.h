#ifndef _MEMORY_H
#define _MEMORY_H

#include "types.h"

/*	GBA memory map available: http://problemkaputt.de/gbatek.htm#gbamemorymap
*/

#define BIT(number) (1<<(number))
#define BIT_OFF(number) (0<<(number))
#define BIT_MASK(from,to) (((1<<(to-from+1))-1)<<from)
#define ALWAYS_INLINE __attribute__((always_inline))
#define ALIGN4 __attribute__((aligned(4)))
#define ARM __attribute__((target("arm")))
#define THUMB __attribute__((target("thumb")))
#define IN_EWRAM __attribute__ ((section (".ewram")))
#define IN_IWRAM __attribute__ ((section (".iwram")))

// BIOS 16kB (no RW)
#define BIOS		0x00000000
#define BIOS_END	0x00003FFF

// unused 0x00004000-0x01FFFFFF

// On-board RAM 256kB (16bit bus)
#define	EWRAM		0x02000000
#define	EWRAM_END	0x0203FFFF

// unused 0x02040000-0x02FFFFFF

// On-chip RAM 32kB (32bit bus)
#define	IWRAM		0x03000000
#define IWRAM_END	0x03007FFF

// unused 0x03008000-0x03FFFFFF

// IO space
#define	IO_BASE		0x04000000
#define IO_END 		0x040003FE

// unused 0x04000400-0x04FFFFFF

// BG/OBJ Palette RAM 1kB
#define PALETTE_RAM	0x05000000
#define PALETTE_RAM_END 0x050003FF

// unused 0x05000400-0x05FFFFFF

// Video RAM 96kB
#define	VRAM		0x06000000
#define OVRAM		0x06010000
#define VRAM_END	0x06017FFF

// unused 0x06018000-06FFFFFF

// OBJ Attributes 1kB
#define OAM			0x07000000
#define OAM_END		0x070003FF

// unused 0x07000400-0x07FFFFFF

// Game Pak ROM/FlashROM (max 32MB)
#define ROM			0x08000000
#define ROM_END		0x09FFFFFF

/*	ROM is mirror over the ranges:
	0x08000000 - 0x09FFFFFF (Wait state 0)
	0x0A000000 - 0x0BFFFFFF (Wait state 1)
	0x0C000000 - 0x0DFFFFFF (Wait state 2)
*/

// Game Pak SRAM (max 64kB)
#define	SRAM		0x0E000000
#define SRAM_END	0x0E00FFFF

// unused 0x0E010000-0xFFFFFFFF

#endif // _MEMORY_H
