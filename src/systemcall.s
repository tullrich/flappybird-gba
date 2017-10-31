@ at top of your file
    .text           @ aka .section .text
    .code 16        @ aka .thumb

@ for each swi (like division, for example)
    .align 2        @ aka .balign 4
    .global SoftReset
    .thumb_func
SoftReset:
	swi		0x00
    bx      lr

    .align 2        @ aka .balign 4
    .global VBlankIntrWait
    .thumb_func
VBlankIntrWait:
	swi		0x05
    bx      lr
