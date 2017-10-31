#ifndef _IRQ_H
#define _IRQ_H

#include "memory.h"

typedef enum IE_IF_BITS {
	IRQ_VBLANK	=	BIT(0),
	IRQ_HBLANK	=	BIT(1),
	IRQ_VCOUNT	=	BIT(2),
	IRQ_TIMER0	=	BIT(3),
	IRQ_TIMER1	=	BIT(4),
	IRQ_TIMER2	=	BIT(5),
	IRQ_TIMER3	=	BIT(6),
	IRQ_COM		=	BIT(7),
	IRQ_DMA0	=	BIT(8),
	IRQ_DMA1	=	BIT(9),
	IRQ_DMA2	=	BIT(10),
	IRQ_DMA3	=	BIT(11),
	IRQ_KEYPAD	=	BIT(12),
	IRQ_GAMEPAK	=	BIT(13)
} IE_IF_BITS;

// Interrupt Enable Register
#define REG_IE *((volatile u16*)(IO_BASE+0x0200))

// Interrupt Request Flags / IRQ Acknowledge
#define REG_IF *((volatile u16*)(IO_BASE+0x0202))

// Game Pak Waitstate Control
#define REG_WAITCNT *((volatile u16*)(IO_BASE+0x0204))

// Interrupt Master Enable Register
// (note that the flag in CPSR must also be enabled, accessible in privileged mode)
#define REG_IME *((volatile u16*)(IO_BASE+0x0208))

// BIOS Inturrupt Acknowledge (resides in IWRAM)
#define REG_IFBIOS *((volatile u16*)(IWRAM+0x7FF8))

// IRQ Handler location in IWRAM that the BIOS will jump to
#define REG_IRQ_VECTOR *((volatile u32*)(IWRAM+0x7FFC))

void irq_init();

#endif // _IRQ_H
