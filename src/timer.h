#ifndef _TIMER_H
#define _TIMER_H

#include "memory.h"

// Current count of the timers 0-3 (R/W).
// Writing does not set the timer value. Rather,
// it sets the initial value for the next timer run.
#define REG_TM0D  *((volatile u16*)(IO_BASE+0x0100))
#define REG_TM1D  *((volatile u16*)(IO_BASE+0x0104))
#define REG_TM2D  *((volatile u16*)(IO_BASE+0x0108))
#define REG_TM3D  *((volatile u16*)(IO_BASE+0x010C))

#define REG_TM0CNT  *((volatile u16*)(IO_BASE+0x0102))
#define REG_TM1CNT  *((volatile u16*)(IO_BASE+0x0106))
#define REG_TM2CNT  *((volatile u16*)(IO_BASE+0x010A))
#define REG_TM3CNT  *((volatile u16*)(IO_BASE+0x010E))

typedef enum TMxCNT_BITS {
	FREQ_DEFAULT	=	0,  // 16.78MHz
	FREQ_64			=	1,
	FREQ_256		=	2,
	FREQ_1024		=	3,
	CASCADE			=	BIT(2),
	IRQ_OFLOW		=	BIT(6),
	TIMER_ENABLE	=	BIT(7)
} TMxCNT_BITS;

typedef struct
{
	u16 count;
	u16 control;
} Timer;

#define timers_mem ((volatile Timer*)(IO_BASE+0x0100))


#endif // _TIMER_H
