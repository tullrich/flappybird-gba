#include "io.h"

#define KEY_MASK 0x03FF

u16 key_state = 0;
u16 key_state_prev = 0;

void key_poll()
{
    key_state_prev= key_state;
    key_state= ~REG_KEYINPUT & KEY_MASK;
}
