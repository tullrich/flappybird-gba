#include "irq.h"
#include "video.h"
#include "audio.h"
#include "dma.h"

ARM IN_IWRAM static void irq_handler() {
	// disable inturrupts
	u32 ime = REG_IME;
	REG_IME = 0;

	u32 iflag = 0;
	/*if (REG_IF & IRQ_VBLANK) {
		irq_counter++;
		//rgb16 c = (31<<((irq_counter%3)*5));
		//m3_plot((100+irq_counter) % M3_WIDTH, 100, c);
		audio_vsync();
		iflag = IRQ_VBLANK;
	} else if (REG_IF & IRQ_HBLANK) {
		if (REG_VCOUNT==0) {
			rgb16 c = (31<<((irq_counter%3)*5));
			m3_draw_rect(0, REG_VCOUNT, M3_WIDTH, 1, c);
		}
		iflag = IRQ_HBLANK;
	}  else if (REG_IF & IRQ_TIMER1) {
		iflag = IRQ_TIMER1;
	}*/


	if (audio_vars.activeBuffer == 1) { // Start playing buffer 0
		dma1_fifo_repeat(REG_FIFO_A, audio_vars.mixBufferBase);
		// Set the current buffer pointer to the start of buffer 1
		audio_vars.curMixBuffer = audio_vars.mixBufferBase + audio_vars.mixBufferSize;
		audio_vars.activeBuffer = 0;
	} else { // buffer 0 just got over
		// DMA points to buffer 1 already, so don't bother stopping and resetting it
		// Set the current buffer pointer to the start of buffer 0
		audio_vars.curMixBuffer = audio_vars.mixBufferBase;
		audio_vars.activeBuffer = 1;
	}
	iflag = IRQ_VBLANK;

	// acknowledge inturrupt
	REG_IF = iflag;
	REG_IFBIOS |= iflag;

	// restore inturrupt enable state
	REG_IME = ime;
}

void irq_init() {
	// setup IRQ
	REG_IRQ_VECTOR = (u32)&irq_handler;
	REG_DISPSTAT = VBL_IRQ | HBL_IRQ;
	REG_IE = IRQ_VBLANK;
	REG_IME = 1;
}
