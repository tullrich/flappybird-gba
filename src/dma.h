#ifndef _DMA_H
#define _DMA_H

#include "memory.h"

// 4 DMA Channels ordered by priority
// 0: Highest priority; use for time critical transfers
// 1: Transfer sound data to sound buffers
// 2: Another one used for transfering sound data to sound buffers
// 3: General-purpose (move bitmap or tile data); lowest priority

// Source/Destination/[Amount,Ctrl] registers
#define REG_DMA0SAD		*((u32*)(IO_BASE+0x00B0))
#define REG_DMA0DAD		*((u32*)(IO_BASE+0x00B4))
#define REG_DMA0CNT		*((u32*)(IO_BASE+0x00B8))
#define REG_DMA0CNT_L	*((u16*)(IO_BASE+0x00B8))
#define REG_DMA0CNT_H	*((u16*)(IO_BASE+0x00BA))

#define REG_DMA1SAD		*((u32*)(IO_BASE+0x00BC))
#define REG_DMA1DAD		*((u32*)(IO_BASE+0x00C0))
#define REG_DMA1CNT		*((u32*)(IO_BASE+0x00C4))
#define REG_DMA1CNT_L	*((u16*)(IO_BASE+0x00C4))
#define REG_DMA1CNT_H	*((u16*)(IO_BASE+0x00C6))

#define REG_DMA2SAD		*((u32*)(IO_BASE+0x00C8))
#define REG_DMA2DAD		*((u32*)(IO_BASE+0x00CC))
#define REG_DMA2CNT		*((u32*)(IO_BASE+0x00D0))
#define REG_DMA2CNT_L	*((u16*)(IO_BASE+0x00D0))
#define REG_DMA2CNT_H	*((u16*)(IO_BASE+0x00D2))

// 28 bit?
#define REG_DMA3SAD		*((u32*)(IO_BASE+0x00D4))
#define REG_DMA3DAD		*((u32*)(IO_BASE+0x00D8))
#define REG_DMA3CNT		*((u32*)(IO_BASE+0x00DC))
#define REG_DMA3CNT_L	*((u16*)(IO_BASE+0x00DC))
#define REG_DMA3CNT_H	*((u16*)(IO_BASE+0x00DE))

typedef enum DNAxCNT_BITS {
	DMA_NUM_TRANSFERS_MASK	=	0xFFFF,
	DMA_DST_INC				=	(0<<21),
	DMA_DST_DEC				=	(1<<21),
	DMA_DST_FIXED			=	(2<<21),
	DMA_DST_RELOAD			=	(3<<21),

	DMA_SRC_INC				=	(0<<23),
	DMA_SRC_DEC				=	(1<<23),
	DMA_SRC_FIXED			=	(2<<23),

	DMA_REPEAT				= 	BIT(25),
	DMA_32					=	BIT(26),

	DMA_NOW					=	(0<<28),
	DMA_AT_VBLANK			=	(1<<28),
	DMA_AT_HBLANK			=	(2<<28),
	DMA_AT_REFRESH			=	(3<<28),

	DMA_IRQ					=	BIT(30),
	DMA_ENABLE				=	BIT(31),

} DNAxCNT_BITS;

// chunks, not bytes! chunks * DMA_32:BIT(26) = bytes
#define DMA_NUM_TRANSFERS(chunks)	(chunks&DMA_NUM_TRANSFERS_MASK)

typedef struct DMAController
{
    const void *src;
    void *dst;
    u32 cnt;
} DMAController;

#define dma_ctrl ((volatile DMAController*)(IO_BASE+0x00B0))

// Utility
inline void dma_fill(void *dst, volatile u32 fill, u32 count, u32 channel, u32 mode)
{
	dma_ctrl[channel].cnt = 0; // shut off any previous transfer
	dma_ctrl[channel].src = (const void*)&fill;
	dma_ctrl[channel].dst = dst;
	dma_ctrl[channel].cnt = DMA_NUM_TRANSFERS(count) | mode | DMA_SRC_FIXED | DMA_ENABLE;
}

inline void dma_copy(void *dst, const void *src, u32 count, u32 channel, u32 mode)
{
	dma_ctrl[channel].cnt = 0; // shut off any previous transfer
	dma_ctrl[channel].src = src;
	dma_ctrl[channel].dst = dst;
	dma_ctrl[channel].cnt = DMA_NUM_TRANSFERS(count) | mode | DMA_ENABLE;
}

inline void dma_fifo_repeat(u32* dst, const void *src, u32 channel)
{
	dma_ctrl[channel].cnt = 0; // shut off any previous transfer
	dma_ctrl[channel].src = src;
	dma_ctrl[channel].dst = dst;
	dma_ctrl[channel].cnt = DMA_AT_REFRESH | DMA_32 | DMA_ENABLE | DMA_SRC_INC
		| DMA_DST_INC | DMA_REPEAT;
}

#define DMA_PROTO(num)																		\
	inline void dma##num##_fill(void *dst, volatile u32 fill, u32 count, u32 mode)			\
	{																						\
		dma_fill(dst, fill, count, num, mode);												\
	}																						\
	inline void dma##num##_copy(void *dst, const void *src, u32 count, u32 mode)			\
	{																						\
		dma_copy(dst, src, count, num, mode);												\
	}																						\
	inline void dma##num##_fifo_repeat(u32* dst, const void *src)							\
	{																						\
		dma_fifo_repeat(dst, src, num);													\
	}																						\

DMA_PROTO(0) // dma0_fill()/dma0_copy()
DMA_PROTO(1) // dma1_fill()/dma1_copy()
DMA_PROTO(2) // ...
DMA_PROTO(3) // ...

#endif // _DMA_H
