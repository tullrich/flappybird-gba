#ifndef _VIDEO_H
#define _VIDEO_H

#include "memory.h"
#include "sin_lut.h"

#define REG_DISPCNT 	*((volatile u32*)(IO_BASE+0x0000))

typedef enum DISPCNT_BITS {
	// BG Mode
	MODE_0	=	0x0000,
	MODE_1	=	0x0001,
	MODE_2	=	0x0002,
	MODE_3	=	0x0003,
	MODE_4	=	0x0004,
	MODE_5	=	0x0005,

	IS_GB	= 	BIT(3),
	PAGE	=	BIT(4),
	OAM_HBL	=	BIT(5),
	OBJ_1D	=	BIT(6),
	BLANK	=	BIT(7),

	// Layers
	BG0_ON	=	BIT(8),
	BG1_ON	=	BIT(9),
	BG2_ON	=	BIT(10),
	BG3_ON	=	BIT(11),
	OBJ_ON	=	BIT(12),

	WIN0_ON	=	BIT(13),
	WIN1_ON	=	BIT(14),
	WINOBJ_ON =	BIT(15),
} DISPCNT_BITS;

#define REG_DISPSTAT 	*((volatile u16*)(IO_BASE+0x0004))

typedef enum DISPSTAT_BITS {
	IN_VBL	=	BIT(0),
	IN_HBL	=	BIT(1),
	IN_VCT	=	BIT(2),
	VBL_IRQ	=	BIT(3),
	HBL_IRQ	=	BIT(4),
	VCT_IRQ	=	BIT(5)
} DISPSTAT_BITS;

#define REG_VCOUNT 	*((volatile u16*)(IO_BASE+0x0006))

#define REG_BG0CNT 	*((volatile u16*)(IO_BASE+0x0008))
#define REG_BG1CNT 	*((volatile u16*)(IO_BASE+0x000A))
#define REG_BG2CNT 	*((volatile u16*)(IO_BASE+0x000C))
#define REG_BG3CNT 	*((volatile u16*)(IO_BASE+0x000E))

typedef enum BGxCNT_BITS {
	BG_PRIORITY_MASK	=	0x3,
	CHAR_BASE_MASK		=	0xC,
	MOSIAC				=	BIT(6),
	PAL_256				=	BIT(7),
	SCREEN_BASE_MASK	=	0x1F00,
	DISP_OVERFLOW		= 	BIT(13),
	SCREEN_SIZE_MASK	= 	0xC000
} BGxCNT_BITS;

#define REG_BG0HOFS 	*((volatile u16*)(IO_BASE+0x0010))
#define REG_BG0VOFS 	*((volatile u16*)(IO_BASE+0x0012))

#define REG_BG1HOFS 	*((volatile u16*)(IO_BASE+0x0014))
#define REG_BG1VOFS 	*((volatile u16*)(IO_BASE+0x0016))

#define REG_BG2HOFS 	*((volatile u16*)(IO_BASE+0x0018))
#define REG_BG2VOFS 	*((volatile u16*)(IO_BASE+0x001A))

#define REG_BG3HOFS 	*((volatile u16*)(IO_BASE+0x001C))
#define REG_BG3VOFS 	*((volatile u16*)(IO_BASE+0x001E))

#define BG_PRIORITY(priority) (priority&BG_PRIORITY_MASK)
#define SET_BG_PRIORITY(bg,priority) (bg&~BG_PRIORITY_MASK)|BG_PRIORITY(priority)
#define BG_CHAR_BASE(base) ((base<<2)&CHAR_BASE_MASK)
#define SET_BG_CHAR_BASE(bg,base) (bg&~CHAR_BASE_MASK)|BG_CHAR_BASE(base)
#define BG_SCREEN_BASE(base) ((base<<8)&SCREEN_BASE_MASK)
#define SET_BG_SCREEN_BASE(bg,base) (bg&~SCREEN_BASE_MASK)|BG_SCREEN_BASE(base)
#define BG_SCREEN_SIZE(size) ((size<<14)&SCREEN_SIZE_MASK)
#define SET_BG_SCREEN_SIZE(bg,size) (bg&~SCREEN_SIZE_MASK)|BG_SCREEN_SIZE(size)

// Screen entry conceptual typedef
typedef u16 ScreenEntry;

#define SCR_ENTRY_TILE_NUM(n) 	(idx&0x03FF)
#define SCR_ENTRY_FLIP_H		(1<<10)
#define SCR_ENTRY_FLIP_V		(1<<11)
#define SCR_ENTRY_PAL_NUM(n)	((idx&0x0F)<<12)

// Screenblock struct
typedef ScreenEntry   ScreenBlock[1024];

#define se_mem ((ScreenBlock*)VRAM)

// 5.5.5 rgb = xBBBBBGGGGGRRRRR
typedef u16 rgb16;
rgb16 RGB16(u32 r, u32 g, u32 b);

// wait for next vblank
void vid_vsync();

// mode 3 vram dimensions
#define M3_WIDTH   240
#define M3_HEIGHT  160

// mode 3 memory
typedef rgb16 M3Line[M3_WIDTH];
#define m3_mem ((M3Line*)VRAM)

// mode 3 utility
void m3_plot(u32 x, u32 y, rgb16 c);
void m3_clear_screen(rgb16 color);
void m3_draw_rect(u32 x, u32 y, u32 sizex, u32 sizey, rgb16 color);
void m3_draw_square(u32 x, u32 y, u32 size, rgb16 color);

// mode 4 vram dimensions
#define M4_WIDTH   240
#define M4_HEIGHT  160

// mode 4 memory 8-bit mode 4 vram pages 0 and 1
#define m4_page0_mem ((u16*)VRAM)
#define m4_page1_mem ((u16*)(VRAM+0xA000))

typedef struct
{
	rgb16 colors[16];
} Palette16;

#define bg_pal256_mem ((rgb16*)PALETTE_RAM)
#define bg_pal16_mem ((Palette16*)PALETTE_RAM)

// mode 4 Utility
void m4_plot(u32 x, u32 y, u8 c_idx);

// Sprites

// tile 8x8@4bpp:
typedef struct
{
	u32 data[8];
} Tile;

// d-tile: double-sized tile (8bpp)
typedef struct {
	u32 data[16];
} Tile8;

// tile block: 32x16 tiles, 16x16 d-tiles
// Each CharBlock is 16kb
typedef Tile  CharBlock[512];
typedef Tile8 CharBlock8[256];

// 96kb of VRAM can be seen as 6 charblocks.
#define tile_mem  ((CharBlock*)VRAM)
#define tile8_mem ((CharBlock8*)VRAM)

typedef struct Palette
{
	rgb16 colors[245];
} Palette;

#define sprite_pal_mem ((rgb16*)(PALETTE_RAM+0x0200))
#define sprite_pal16_mem ((Palette16*)(PALETTE_RAM+0x0200))

typedef struct ObjAttr
{
	u16 attrib[3];
	u16 dummy;
} ALIGN4 ObjAttr;

#define A0_Y_COORD(y)	((y)&0x00FF)
#define A0_ROT_SCALE_ON	(1<<8)
// When Rotation/Scaling used
#define A0_SIZE_DOUBLE 	(1<<9)
// When Rotation/Scaling not used
#define A0_HIDE		(1<<9)
#define A0_NORMAL	(0<<10)
#define A0_SEMI_TRANSPARENT	(1<<10)
#define A0_OBJ_WINDOW	(2<<10)
// Bits E-F become the top 2bits of a 4bits number combined with A1[E-F]
#define A0_SQUARE	(0<<14)
#define A0_WIDE		(1<<14)
#define A0_TALL		(2<<14)
#define A0_PAL_256	BIT(13)

#define A1_X_COORD(x)	((x)&0x01FF)
#define A1_SIZE_8		(0<<14)
#define A1_SIZE_16		(1<<14)
#define A1_SIZE_32		(2<<14)
#define A1_SIZE_64		(3<<14)
#define A1_AFF_ID(id) 	((id&0x1F)<<9)

#define A2_TILE_NUM(n)	((n)&0x03FF)
#define A2_PRIORITY(n)	(((n)&0x03)<<10)
#define A2_PAL_NUM(n)	(((n)&0x0F)<<12)

// ObjAttr
typedef enum ObjAttrSize {
	OA_SIZE_8x8		= 0x00,
	OA_SIZE_16x16	= 0x01,
	OA_SIZE_32x32	= 0x02,
	OA_SIZE_64x64	= 0x03,
	OA_SIZE_16x8	= 0x04,
	OA_SIZE_32x8	= 0x05,
	OA_SIZE_32x16	= 0x06,
	OA_SIZE_64x32	= 0x07,
	OA_SIZE_8x16	= 0x08,
	OA_SIZE_8x32	= 0x09,
	OA_SIZE_16x32	= 0x0A,
	OA_SIZE_32x64	= 0x0B
} ObjAttrSize;

inline void obj_attr_hide(ObjAttr* obj) {
	obj->attrib[0] = (obj->attrib[1]&~A0_ROT_SCALE_ON)|A0_HIDE;
}

inline void obj_attr_show(ObjAttr* obj) {
	obj->attrib[0] &= ~A0_HIDE;
}

inline void obj_attr_set_priority(ObjAttr* obj, u32 priority) {
	obj->attrib[2] = (obj->attrib[2]&~0x03)|A2_PRIORITY(priority);
}

inline void obj_attr_set_position(ObjAttr* obj, u32 x, u32 y) {
	obj->attrib[0] = (obj->attrib[0]&~0x00FF)|A0_Y_COORD(y);
	obj->attrib[1] = (obj->attrib[1]&~0x01FF)|A1_X_COORD(x);
}

inline void obj_attr_set_size(ObjAttr* obj, ObjAttrSize size) {
	obj->attrib[0] = (obj->attrib[0]&0x3FFF)|((size&0x0C)<<12);
	obj->attrib[1] = (obj->attrib[1]&0x3FFF)|((size&0x03)<<14);
}

inline void obj_attr_init(ObjAttr* obj, u32 x, u32 y, ObjAttrSize size, u32 tile_base, u32 pal) {
	obj->attrib[0] = ((size&0x0C)<<12)|A0_Y_COORD(y);
	obj->attrib[1] = ((size&0x03)<<14)|A1_X_COORD(x);
	obj->attrib[2] = A2_TILE_NUM(tile_base)|A2_PAL_NUM(pal);
}

inline void obj_attr_set_flags(ObjAttr* obj, u32 a0, u32 a1, u32 a2) {
	obj->attrib[0] |= a0;
	obj->attrib[1] |= a1;
	obj->attrib[2] |= a2;
}

inline void obj_attr_set_tile_base(ObjAttr* obj, u32 tile_base) {
	obj->attrib[2] = (obj->attrib[2]&~0x3FF)|A2_TILE_NUM(tile_base);
}

#define oam_mem ((ObjAttr*)OAM)
#define OAM_OBJ_SIZE 128

void oam_init(ObjAttr *obj, u32 count);
void oam_copy(ObjAttr *dst, const ObjAttr *src, u32 count);

typedef struct ObjAffine
{
    u16 fill0[3];
    s16 pa;
    u16 fill1[3];
    s16 pb;
    u16 fill2[3];
    s16 pc;
    u16 fill3[3];
    s16 pd;
} ALIGN4 ObjAffine;

inline void obj_aff_identity(ObjAffine *oaff) {
	oaff->pa = oaff->pd = 1<<8;
	oaff->pb = oaff->pc = 0<<8;
}

inline void obj_aff_scale(ObjAffine *oaff, u32 scale_x, u32 scale_y) {
    oaff->pa = (oaff->pa<<12) / scale_x;
	oaff->pb = (oaff->pb<<12) / scale_x;
    oaff->pc = (oaff->pc<<12) / scale_y;
	oaff->pd = (oaff->pd<<12) / scale_y;
}

inline void obj_aff_rotscale(ObjAffine *oaff, u32 scale_x, u32 scale_y, u16 angle) {
    s16 ss = (lu_sin(angle)-0x4000)>>6, cc = (lu_cos(angle)-0x4000)>>6;
    oaff->pa= cc;//*scale_x>>12;
	oaff->pb=-ss;//*scale_x>>12;
    oaff->pc= ss;//*scale_y>>12;
	oaff->pd= cc;//*scale_y>>12;
}

#define oam_affine_mem ((ObjAffine*)OAM)
#define OAM_AFFINE_MEM_SIZE 32

// Windowing registers

// Window 1/2 horizontal positions (Write Only)
#define REG_WIN0H	  	*((volatile u16*)(IO_BASE+0x0040))
#define REG_WIN1H	  	*((volatile u16*)(IO_BASE+0x0042))

#define WINxH_RIGHT(r) (r&0xFF)
#define WINxH_LEFT(l) ((l&0xFF)<<8)

// Window 1/2 vertical positions (Write Only)
#define REG_WIN0V	  	*((volatile u16*)(IO_BASE+0x0044))
#define REG_WIN1V	  	*((volatile u16*)(IO_BASE+0x0046))

#define WINxH_BOTTOM(b) (b&0xFF)
#define WINxH_TOP(t) ((t&0xFF)<<8)

// Union of Window 1,2 settings
#define REG_WININ	  	*((volatile u16*)(IO_BASE+0x0048))
#define REG_WINOUT	  	*((volatile u16*)(IO_BASE+0x004A))

typedef enum WININ_BITS {
	BG0_WIN0 		= BIT(0),
	BG1_WIN0 		= BIT(1),
	BG2_WIN0 		= BIT(2),
	BG3_WIN0 		= BIT(3),
	OBJ_WIN0 		= BIT(4),
	BLENDS_WIN0 	= BIT(5),
	BG0_WIN1 		= BIT(8),
	BG1_WIN1 		= BIT(9),
	BG2_WIN1 		= BIT(10),
	BG3_WIN1 		= BIT(11),
	OBJ_WIN1 		= BIT(12),
	BLENDS_WIN1 	= BIT(13),
} WININ_BITS;

// Bits for REG_WININ
typedef enum WINOUT_BITS {
	BG0_OUT 		= BIT(0),
	BG1_OUT 		= BIT(1),
	BG2_OUT 		= BIT(2),
	BG3_OUT 		= BIT(3),
	OBJ_OUT 		= BIT(4),
	BLENDS_OUT 		= BIT(5),
	BG0_OBJ 		= BIT(8),
	BG1_OBJ 		= BIT(9),
	BG2_OBJ 		= BIT(10),
	BG3_OBJ 		= BIT(11),
	OBJ_OBJ 		= BIT(12),
	BLENDS_OBJ 		= BIT(13),
} WINOUT_BITS;

inline void window0_set_dimension(u32 left, u32 right, u32 top, u32 bottom) {
	REG_WIN0H = WINxH_LEFT(left)|WINxH_RIGHT(right);
	REG_WIN0V = WINxH_BOTTOM(bottom)|WINxH_TOP(top);
}

inline void window1_set_dimension(u32 left, u32 right, u32 top, u32 bottom) {
	REG_WIN1H = WINxH_LEFT(left)|WINxH_RIGHT(right);
	REG_WIN1V = WINxH_BOTTOM(bottom)|WINxH_TOP(top);
}

// Effect registers

// Controls the size of the mosaic on backgrounds/sprites that have mosaic enabled
// (Write Only)
#define REG_MOSAIC	  	*((volatile u32*)(IO_BASE+0x004C))

// Determines the blending mode and which layer(s) you wish to perform blending on
#define REG_BLDCNT 		*((volatile u16*)(IO_BASE+0x0050))

typedef enum BLDCNT_BITS {
	BLEND_BG0_SRC		=	BIT(0),
	BLEND_BG1_SRC		=	BIT(1),
	BLEND_BG2_SRC		=	BIT(2),
	BLEND_BG3_SRC		=	BIT(3),
	BLEND_SPRITES_SRC	=	BIT(4),
	BLEND_BACKDROP_SRC	=	BIT(5),
	BLEND_MODE_OFF		=	(0<<6),
	BLEND_MODE_ALPHA	=	(1<<6),
	BLEND_MODE_LIGHTEN	=	(2<<6),
	BLEND_MODE_DARKEN	=	(3<<6),
	BLEND_BG0_DST		=	BIT(8),
	BLEND_BG1_DST		=	BIT(9),
	BLEND_BG2_DST		=	BIT(10),
	BLEND_BG3_DST		=	BIT(11),
	BLEND_SPRITES_DST	=	BIT(12),
	BLEND_BACKDROP_DST	=	BIT(13),
} BLDCNT_BITS;

// Amount of alpha blending between layers (Write Only)
#define REG_COLEV 		*((volatile u16*)(IO_BASE+0x0052))

// An unblended pixel of normal intensity is is considered to have a coefficient
// of 16. Coefficient A and Coefficient B determine the ratio of each of the
// sources that will get mixed into the final image. Thus, if A is 12 and B
// is 4, the resulting image will appear to be 12/16 the color of A and 4/16
// the color of B. Note that A and B can add up to be greater than 16 (for an
// additive or brightening effect) or less than 16 (for a darkening effect).
#define SET_COLEV_SRC_ALPHA(a) (a&0x1F)
#define SET_COLEV_DST_ALPHA(a) ((a&0x1F)<<8)

// Amount by which to lighten or darken the source layers (Write Only)
#define REG_COLEY 		*((volatile u16*)(IO_BASE+0x0054))
#endif //_VIDEO_H
