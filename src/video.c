#include "video.h"
#include "dma.h"

rgb16 RGB16(u32 r, u32 g, u32 b) {
	return r | (g<<5) | (b<<10);
}

void vid_vsync() {
    while(REG_VCOUNT >= 160);   // wait till VDraw
    while(REG_VCOUNT < 160);    // wait till VBlank
}

void m3_plot(u32 x, u32 y, rgb16 c) {
	((u16*)VRAM)[y*M3_WIDTH+x] = c;
}

void m3_clear_screen(rgb16 color) {
	//TODO: write a u32 at a time
	for (u32 row = 0; row < M3_HEIGHT; row++) {
        for (u32 col = 0; col < M3_WIDTH; col++) {
            m3_plot(col, row, color);
        }
    }
}

void m3_draw_rect(u32 x, u32 y, u32 sizex, u32 sizey, rgb16 color) {
    for (u32 row = y; row < (y + sizey); row++) {
        for (u32 col = x; col < (x + sizex); col++) {
            m3_plot(col, row, color);
        }
    }
}

void m3_draw_square(u32 x, u32 y, u32 size, rgb16 color) {
	m3_draw_rect(x, y, size, size, color);
}


void m4_plot(u32 x, u32 y, u8 c_idx) {
	u16 *dst= &m4_page0_mem[(y*M4_WIDTH+x)/2];  // Division by 2 due to u8/u16 pointer mismatch!
    if(x&1)
        *dst= (*dst& 0xFF) | (c_idx<<8);    // odd pixel
    else
        *dst= (*dst&~0xFF) |  c_idx;        // even pixel
}

void oam_init(ObjAttr *obj, u32 count) {
    u32 nn= count;
    u32 *dst= (u32*)obj;

    // Hide each object
    while(nn--) {
        *dst++= A0_HIDE;
        *dst++= 0;
    }

    // init oam
    oam_copy(oam_mem, obj, count);
}

void oam_copy(ObjAttr *dst, const ObjAttr *src, u32 count) {
	dma3_copy(dst, src, count*sizeof(ObjAttr)/4, DMA_NOW|DMA_32);
}
