#ifndef _IO_H
#define _IO_H

#include "memory.h"

// Key Status (inverted; default state 0x03FF)
#define REG_KEYINPUT *((volatile u16*)(IO_BASE+0x0130))

// Key Interrupt Control
#define REG_KEYCNT *((volatile u16*)(IO_BASE+0x0132))

typedef enum KEY_BITS {
	// KEYINPUT and KEYCNT
	KEY_A			=	BIT(0), // 0=Pressed, 1=Released
	KEY_B			=	BIT(1),
	KEY_SELECT		=	BIT(2),
	KEY_START		=	BIT(3),
	KEY_RIGHT		=	BIT(4),
	KEY_LEFT		=	BIT(5),
	KEY_UP			=	BIT(6),
	KEY_DOWN		=	BIT(7),
	KEY_R			=	BIT(8),
	KEY_L			=	BIT(9),

	// KEYCNT only
	KEY_IRQ_ENABLE	=	BIT(14), // (0=Disable, 1=Enable)
	KEY_IRQ_COND	=	BIT(15), // (0=Logical OR, 1=Logical AND)

} KEY_BITS;

extern u16 key_state;
extern u16 key_state_prev;

// Utility
void key_poll();
inline u32 key_curr_state()         {   return key_state;          		}
inline u32 key_prev_state()         {   return key_state_prev;        	}
inline u32 key_is_down(u32 key)     {   return key_state&key;   		}
inline u32 key_is_up(u32 key)       {   return ~key_state&key;   		}
inline u32 key_was_down(u32 key)    {   return key_state_prev&key;  	}
inline u32 key_was_up(u32 key)      {   return ~key_state_prev&key;   	}

inline u32 key_changed(u32 key)		{	return key&(key_state^key_state_prev); 	}
inline u32 key_held(u32 key)		{ 	return key&(key_state&key_state_prev);	}
inline u32 key_hit(u32 key)			{	return key&(key_state&~key_state_prev); }
inline u32 key_released(u32 key)	{	return key&(~key_state&key_state_prev); }

#endif //_IO_H
