#include "types.h"
#include "memory.h"
#include "io.h"
#include "irq.h"
#include "video.h"
#include "dma.h"
#include "timer.h"
#include "audio.h"
#include "systemcall.h"

#include "flappy_background.h"
#include "flappy_foreground.h"
#include "flappy_sprites.h"

#include <string.h>
#include <stdlib.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define clamp(a, _min, _max) min(max(a, _min), _max)

// Player defs
#define PLAYER_MAX_VELOCITY (3<<12) // (20.12 fixed point)
#define PLAYER_ACCELERATION -669 // 9.8/60 = ~.163 = 669/4096 (20.12 fixed point)
#define PLAYER_X_OFFSET 40 // pixels
#define PLAYER_DIMS 32 // pixels
#define PLAYER_ANIMATION_SPEED 8 // frames

// Pipe defs
#define PIPE_TILE_DIM_X 4
#define PIPE_TILE_DIM_Y 16
#define PIPE_DIM_X (PIPE_TILE_DIM_X*8)
#define PIPE_DIM_Y (PIPE_TILE_DIM_Y*8)
#define PIPE_PIXEL_GAP 48
#define PIPE_SPEED (1<<12)

// Tile VRAM layout
//<-------VRAM(0x06000000)--->
//<-------Charblock0(512 tiles)(0x06000000)--->
//<xxxxxBG1(83 tiles)[0x0000-0x0A60]xxx>
//<-------Charblock1(512 tiles)(0x06004000)--->
//<xxxxxBG0(9 tiles)[0x4000-0x4120]xxx>
//<-------Charblock2(512 tiles)(0x06008000)--->
//<-------Charblock3(512 tiles)(0x0600C000)--->
//<xBG0_Map(18x32)[0xF000-0xF800]x>
//<xBG1_Map(18x32)[0xF800-0xFC80]x>
//<------OVRAM(0x06010000)--->
//<-------Charblock4(512 tiles)[0x06010000]--->
//<xxxxxBird_Sprite(64 tiles)[0x0000-0x0800]xxx>
//<xxxxxPipe_Sprite(128 tiles)[0x4800-0x5800]xxx>
//<xxxxxButton_Sprite(60 tiles)[0x5800-0x5F80]xxx>
//<xxxxxText_Sprite(144 tiles)[0x5F80-0x7180]xxx>
//<-------Charblock5(512 tiles)[0x06014000]--->
//<---VRAM_END(0x06017FFF)--->
#define BIRD_TILE_BASE 0
#define BIRD_TILE_SIZE 64
#define BIRD_PAL_IDX 0
#define PIPE_TILE_BASE (BIRD_TILE_BASE+BIRD_TILE_SIZE)
#define PIPE_TILE_SIZE 128
#define PIPE_PAL_IDX 1
#define BTN_TILE_BASE (PIPE_TILE_BASE+PIPE_TILE_SIZE)
#define BTN_TILE_SIZE 60
#define BTN_PAL_IDX 2
#define TEXT_TILE_BASE (BTN_TILE_BASE+BTN_TILE_SIZE)
#define TEXT_TILE_SIZE 144
#define TEXT_PAL_IDX 3

// OAM layout
//<--------OAM(0x07000000)--->
//<xPlayer[0][0x0000]x>
//<Pipe[1][0x0008]x>
//<Pipe[2][0x0010]x>
//<Pipe[3][0x0018]x>
//<Pipe[4][0x0020]x>
//<----OAM_END(0x070003FF)--->
#define BTN_OBJECT_IDX1 0
#define BTN_OBJECT_IDX2 1
#define BTN_OBJECT_IDX3 2
#define TEXT_OBJECT_IDX1 3
#define TEXT_OBJECT_IDX2 4
#define TEXT_OBJECT_IDX3 5
#define PLAYER_OBJECT_IDX 6
#define PIPE_OBJECT_IDX1 7
#define PIPE_OBJECT_IDX2 8
#define PIPE_OBJECT_IDX3 9
#define PIPE_OBJECT_IDX4 10

// Player sprite tile animation offsets
static const u32 player_flap_anim[4] = { 16, 32, 48, 32 };

// The player character
typedef struct Player {
	ObjAttr	*sprite; 	// hw renderable
	ObjAffine *tform;	// transform
	s32 vel_y;			// velocity y (20.12 fixed point)
	s32 pos_x, pos_y;	// position x,y (20.12 fixed point)
} Player;

typedef struct Pipe {
	ObjAttr	*sprite_upper[2], *sprite_lower[2]; 	// hw renderables
	s32 pos_x, pos_y;	// position x,y (20.12 fixed point)
} Pipe;

typedef enum GameState {
	GAME_STATE_MENU,
	GAME_STATE_PLAYING,
	GAME_STATE_DEAD,
} GameState;

// The player instance
Player player;

// Pipe sprite
Pipe pipe;

// Background scroll offset
s32 bg_off;

// Game mode
u32 game_mode, next_game_mode;

// The players score
u32 player_score;

// Frame number
u32 frame;

// game pause
u8 game_paused;

ObjAttr objects[OAM_OBJ_SIZE];
ObjAffine *objects_affine= (ObjAffine*)objects;

// Audio binary data
extern u8 _binary_sfx_die_raw_start;
extern u8 _binary_sfx_die_raw_size;
extern u8 _binary_sfx_wing_raw_start;
extern u8 _binary_sfx_wing_raw_size;
extern u8 _binary_sfx_hit_raw_start;
extern u8 _binary_sfx_hit_raw_size;
extern u8 _binary_sfx_swooshing_raw_start;
extern u8 _binary_sfx_swooshing_raw_size;
extern u8 _binary_sfx_point_raw_start;
extern u8 _binary_sfx_point_raw_size;

static int aabb_aabb_collision(s32 ax, s32 ay, s32 aw, s32 ah, s32 bx, s32 by, s32 bw, s32 bh) {
	s32 ar = ax + aw;
	s32 br = bx + bw;
	s32 ab = ay + ah;
	s32 bb = by + bh;
	if (ax <= br && ar >= bx) {
		if (ay <= bb && ab >= by) {
			return 1;
		}
	}
	return 0;
}

// Initialize BG1
static void init_background() {
	// Setup VRAM pointers
	REG_BG1CNT = BG_CHAR_BASE(0)|BG_SCREEN_BASE(31)|BG_PRIORITY(1);

	// Offset the screen down in the vertical
	REG_BG1VOFS = 90;

	// Load tile VRAM
	memcpy(&tile_mem[0][0], flappy_backgroundTiles, flappy_backgroundTilesLen);

	// Load palette into bg pal16 1
	memcpy(&bg_pal16_mem[1], flappy_backgroundPal, flappy_backgroundPalLen);

	// Load map into VRAM
	for (u32 y = 0; y < 32; y++) {
		// First 18 tiles on a row
		memcpy(&se_mem[31][y*32], &flappy_backgroundMap[y*18], sizeof(u16)*18);

		// Repat the first 14 for the remainder of the row
		memcpy(&se_mem[31][y*32+18], &flappy_backgroundMap[y*18], sizeof(u16)*14);
	}
}

// Initialize BG0
static void init_foreground() {
	// Setup VRAM pointers
	REG_BG0CNT = BG_CHAR_BASE(1)|BG_SCREEN_BASE(30)|BG_PRIORITY(0);

	// Load tile VRAM
    memcpy(&tile_mem[1][0], flappy_foregroundTiles, flappy_foregroundTilesLen);

	// Load palette into bgpal16 0
	memcpy(&bg_pal16_mem[0], flappy_foregroundPal, flappy_foregroundPalLen);

	// Load map into VRAM
	for (u32 y = 0; y < 7; y++) {
		dma3_copy(&se_mem[30][(y+16)*32], &flappy_foregroundMap[y*7], 8, DMA_NOW);
		dma3_copy(&se_mem[30][(y+16)*32+7], &flappy_foregroundMap[y*7], 8, DMA_NOW);
		dma3_copy(&se_mem[30][(y+16)*32+14], &flappy_foregroundMap[y*7], 8, DMA_NOW);
		dma3_copy(&se_mem[30][(y+16)*32+21], &flappy_foregroundMap[y*7], 8, DMA_NOW);
		dma3_copy(&se_mem[30][(y+16)*32+28], &flappy_foregroundMap[y*7], 4, DMA_NOW);
	}

	// Clear background scroll offset
	bg_off = 0;
}

// Initialize Sprite VRAM (Charbocks 4/5)
static void init_sprite_vram() {
	// Bird sprites
	dma3_copy(&tile_mem[4][BIRD_TILE_BASE], flappy_bird_spriteTiles, flappy_bird_spriteTilesLen/2, DMA_NOW);
	memcpy(&sprite_pal16_mem[BIRD_PAL_IDX], flappy_bird_spritePal, flappy_bird_spritePalLen);

	// Pipe sprites
	dma3_copy(&tile_mem[4][PIPE_TILE_BASE], flappy_pipe_spriteTiles, flappy_pipe_spriteTilesLen/2, DMA_NOW);
    memcpy(&sprite_pal16_mem[PIPE_PAL_IDX], flappy_pipe_spritePal, flappy_pipe_spritePalLen);

	// Button sprites
	dma3_copy(&tile_mem[4][BTN_TILE_BASE], flappy_buttons_spriteTiles, flappy_buttons_spriteTilesLen/2, DMA_NOW);
	memcpy(&sprite_pal16_mem[BTN_PAL_IDX], flappy_buttons_spritePal, flappy_buttons_spritePalLen);

	// Text sprites
	dma3_copy(&tile_mem[4][TEXT_TILE_BASE], flappy_text_spriteTiles, flappy_text_spriteTilesLen/2, DMA_NOW);
	memcpy(&sprite_pal16_mem[TEXT_PAL_IDX], flappy_text_spritePal, flappy_text_spritePalLen);
}

// Initialize plauer sprite
static void init_player() {
	//	Init player object
	memset(&player, 0, sizeof(Player));
	player.pos_x = ((M4_WIDTH - PLAYER_DIMS)/2 - PLAYER_X_OFFSET)<<12;
	player.pos_y = ((M4_HEIGHT - PLAYER_DIMS + 16)/2)<<12;

	// Init obj attr
	ObjAttr *sprite = player.sprite = &objects[PLAYER_OBJECT_IDX];
	obj_attr_init(sprite, player.pos_x>>12, player.pos_y>>12, OA_SIZE_32x32, BIRD_TILE_BASE+player_flap_anim[0], 0);
	obj_attr_set_flags(sprite, A0_ROT_SCALE_ON, 0, 0);

	// Init obj 2x2 affine matrix
	ObjAffine *tform = player.tform = &objects_affine[0];
	obj_aff_identity(tform);
}

// Initialize Pipe sprite
static void init_pipe() {
	memset(&pipe, 0, sizeof(Pipe));
	pipe.pos_x = M4_WIDTH<<12;
	// [-120-PIPE_PIXEL_GAP, -8+PIPE_PIXEL_GAP]
	pipe.pos_y = -((rand()%(112-PIPE_PIXEL_GAP)+8+PIPE_PIXEL_GAP)<<12);

	ObjAttr *pipe_upper = pipe.sprite_upper[0] = &objects[PIPE_OBJECT_IDX1];
	obj_attr_init(pipe_upper, pipe.pos_x>>12, pipe.pos_y>>12, OA_SIZE_32x64, PIPE_TILE_BASE, 1);
	obj_attr_set_priority(pipe_upper, 1);

	ObjAttr *pipe_upper2 = pipe.sprite_upper[1] = &objects[PIPE_OBJECT_IDX2];
	obj_attr_init(pipe_upper2, pipe.pos_x>>12, (pipe.pos_y>>12)+64, OA_SIZE_32x64, PIPE_TILE_BASE+32, 1);
	obj_attr_set_priority(pipe_upper2, 1);

	ObjAttr *pipe_lower = pipe.sprite_lower[0] = &objects[PIPE_OBJECT_IDX3];
	obj_attr_init(pipe_lower, pipe.pos_x>>12, (pipe.pos_y>>12)+128+PIPE_PIXEL_GAP, OA_SIZE_32x64, PIPE_TILE_BASE+64, 1);
	obj_attr_set_priority(pipe_lower, 1);

	ObjAttr *pipe_lower2 = pipe.sprite_lower[1] = &objects[PIPE_OBJECT_IDX4];
	obj_attr_init(pipe_lower2, pipe.pos_x>>12, (pipe.pos_y>>12)+64+128+PIPE_PIXEL_GAP, OA_SIZE_32x64, PIPE_TILE_BASE+96, 1);
	obj_attr_set_priority(pipe_lower2, 1);
}

// Jump logic
static void player_jump() {
	player.vel_y = PLAYER_MAX_VELOCITY;
	audio_play(0
		, (s8*)&_binary_sfx_wing_raw_start
		, (u32)&_binary_sfx_wing_raw_size
		, 10512, 2, 0 );
}

// Player input
static void update_controls() {
	if (key_is_down(KEY_L)) {
		player.pos_x--;
	}
	if (key_is_down(KEY_R)) {
		player.pos_x++;
	}
	if (key_hit(KEY_START)) {
		game_paused = !game_paused;
		audio_play(1
			, (s8*)&_binary_sfx_die_raw_start
			, (u32)&_binary_sfx_die_raw_size
			, 10512, 2, 0 );
	}
	if (key_hit(KEY_B)) {
		player_jump();
	}
}

// Advance player state each frame
static void update_player(u32 anim_only) {
	ObjAttr* sprite = player.sprite;

	// advance animation frame (0-3)
	u32 animFrame = (frame/PLAYER_ANIMATION_SPEED)%4;
	obj_attr_set_tile_base(sprite, BIRD_TILE_BASE+player_flap_anim[animFrame]);

	// Bird physics
	if (!anim_only && !game_paused) {
		// Apply gravity
		player.vel_y = clamp(player.vel_y+PLAYER_ACCELERATION, -PLAYER_MAX_VELOCITY, PLAYER_MAX_VELOCITY);

		// Apply player velocity
		player.pos_y = clamp(player.pos_y-player.vel_y, -12<<12, M4_HEIGHT<<12);
		if (player.pos_y<=(-12<<12)) { // hit top, lose velocity
			player.vel_y = 0;
		}

		// Update obj attr
		obj_attr_set_position(sprite, player.pos_x>>12, player.pos_y>>12);

		// Update rotation
		u32 angle = 90*player.vel_y/PLAYER_MAX_VELOCITY;
		obj_aff_rotscale(player.tform, 1<<12, 1<<12, angle);

		// Bird x Ground collision
		if (player.pos_y > (104<<12)) {
			next_game_mode = GAME_STATE_DEAD;
			return;
		}

		// Bird x Pipe upper collision
		if (aabb_aabb_collision(player.pos_x+(8<<12), player.pos_y+(8<<12), 16<<12, 16<<12
				, pipe.pos_x, pipe.pos_y, 32<<12, 128<<12)) {
			next_game_mode = GAME_STATE_DEAD;
			return;
		}

		// Bird x Pipe lower collision
		if (aabb_aabb_collision(player.pos_x+(8<<12), player.pos_y+(8<<12), 16<<12, 16<<12
				, pipe.pos_x, pipe.pos_y+((128+PIPE_PIXEL_GAP)<<12), 32<<12, 64<<12)) {
			next_game_mode = GAME_STATE_DEAD;
			return;
		}

		// Did we pass the pipe this frame?
		if (player.pos_x < pipe.pos_x+PIPE_SPEED && player.pos_x >= pipe.pos_x) {
			player_score++;
			audio_play(3
				, (s8*)&_binary_sfx_point_raw_start
				, (u32)&_binary_sfx_point_raw_size
				, 10512, 2, 0 );
		}
	}
}

// Move pipe and reset upon falling off the left edge
static void update_pipe() {
	if (!game_paused) {
		// Move left
		pipe.pos_x -= PIPE_SPEED;

		// Reset to just off the right edge once we leave the left edge
		if (((pipe.pos_x>>12) + PIPE_DIM_X) <= 0) {
			init_pipe(); // reset pipe
		}
	}

	// Update hw object
	obj_attr_set_position(pipe.sprite_upper[0], pipe.pos_x>>12, (pipe.pos_y>>12));
	obj_attr_set_position(pipe.sprite_upper[1], pipe.pos_x>>12, (pipe.pos_y>>12)+64);
	obj_attr_set_position(pipe.sprite_lower[0], pipe.pos_x>>12, (pipe.pos_y>>12)+128+PIPE_PIXEL_GAP);
	obj_attr_set_position(pipe.sprite_lower[1], pipe.pos_x>>12, (pipe.pos_y>>12)+64+128+PIPE_PIXEL_GAP);
}

// Scroll backtground and foreground tilemaps via hw
static void update_background() {
	if (!game_paused) {
		REG_BG0HOFS = ++bg_off;
		REG_BG1HOFS = bg_off/4;
	}
}

// Enter menu state logic
static void set_game_state_menu() {
	// Reset background
	bg_off = 0;
	REG_BG0HOFS = 0;
	REG_BG1HOFS = 0;

	init_player();
	init_pipe();

	u32 btn_pos_x = (M4_WIDTH-6*8)/2;
	u32 btn_pos_y = (M4_HEIGHT)/2;

	ObjAttr *start_btn1 = &objects[BTN_OBJECT_IDX1];
	obj_attr_init(start_btn1, btn_pos_x, btn_pos_y, OA_SIZE_16x16, BTN_TILE_BASE+48, 2);

	ObjAttr *start_btn2 = &objects[BTN_OBJECT_IDX2];
	obj_attr_init(start_btn2, btn_pos_x+16, btn_pos_y, OA_SIZE_16x16, BTN_TILE_BASE+52, 2);

	ObjAttr *start_btn3 = &objects[BTN_OBJECT_IDX3];
	obj_attr_init(start_btn3, btn_pos_x+32, btn_pos_y, OA_SIZE_16x16, BTN_TILE_BASE+56, 2);

	u32 text_pos_x = (M4_WIDTH-12*8)/2;
	u32 text_pos_y = (M4_HEIGHT-8*8)/2;

	ObjAttr *start_text1 = &objects[TEXT_OBJECT_IDX1];
	obj_attr_init(start_text1, text_pos_x, text_pos_y, OA_SIZE_32x32, TEXT_TILE_BASE, 3);

	ObjAttr *start_text2 = &objects[TEXT_OBJECT_IDX2];
	obj_attr_init(start_text2, text_pos_x+32, text_pos_y, OA_SIZE_32x32, TEXT_TILE_BASE+16, 3);

	ObjAttr *start_text3 = &objects[TEXT_OBJECT_IDX3];
	obj_attr_init(start_text3, text_pos_x+64, text_pos_y, OA_SIZE_32x32, TEXT_TILE_BASE+32, 3);
}

// Enter playing state logic
static void set_game_state_playing() {
	// Hide start game text and button
	for (u32 i = BTN_OBJECT_IDX1; i <= TEXT_OBJECT_IDX3; i++) {
		obj_attr_hide(&objects[i]);
	}

	// Reset score
	player_score = 0;

	// Start with a jump
	player_jump();
}

// Enter death state logic
static void set_game_state_dead() {
	// Set gamover text
	u32 text_pos_x = (M4_WIDTH-12*8)/2;
	u32 text_pos_y = (M4_HEIGHT-8*8)/2;
	ObjAttr *game_over_text1 = &objects[TEXT_OBJECT_IDX1];
	obj_attr_init(game_over_text1, text_pos_x, text_pos_y, OA_SIZE_32x32, TEXT_TILE_BASE+48, 3);
	ObjAttr *game_over_text2 = &objects[TEXT_OBJECT_IDX2];
	obj_attr_init(game_over_text2, text_pos_x+32, text_pos_y, OA_SIZE_32x32, TEXT_TILE_BASE+48+16, 3);
	ObjAttr *game_over_text3 = &objects[TEXT_OBJECT_IDX3];
	obj_attr_init(game_over_text3, text_pos_x+64, text_pos_y, OA_SIZE_32x32, TEXT_TILE_BASE+48+32, 3);

	// Play death sound
	audio_play(2
		, (s8*)&_binary_sfx_hit_raw_start
		, (u32)&_binary_sfx_hit_raw_size
		, 10512, 2, 0 );
}

int main(int argc, char* argv[]) {
	// Init DirectSound mixer
	audio_init();

	// Setup PPU video mode 0 (tile map)
	REG_DISPCNT = MODE_0
		| BG0_ON | BG1_ON
		| OBJ_ON | OBJ_1D;

	// Setup blending
	REG_BLDCNT = BLEND_MODE_ALPHA|BLEND_BG0_DST|BLEND_BG1_DST;
	REG_COLEV = SET_COLEV_SRC_ALPHA(8)|SET_COLEV_DST_ALPHA(8);

	// Init inturrupt handler
	irq_init();

	// Init sprite OAM and ERAM shadow
	oam_init(objects, OAM_OBJ_SIZE);

	// One-time game init
	init_background();
	init_foreground();
	init_sprite_vram();

	// Enter initial state
	game_mode = next_game_mode = GAME_STATE_MENU;
	set_game_state_menu();

	frame = 0;
	while(1) {
		// CPU halt until vblank interval
		VBlankIntrWait();

		// Poll keystate
		key_poll();

		// Mix audio
		audio_mix();

		// Switch game state
		if (game_mode != next_game_mode) {
			game_mode = next_game_mode;
			switch (game_mode) {
				case GAME_STATE_MENU: set_game_state_menu(); break;
				case GAME_STATE_PLAYING: set_game_state_playing(); break;
				case GAME_STATE_DEAD: set_game_state_dead(); break;
			}
		}

		// State specific update
		if (game_mode == GAME_STATE_MENU) {
			// Update player anim only
			update_player(1);

			if (key_hit(KEY_B)) {
				next_game_mode = GAME_STATE_PLAYING;
			}
		} else if (game_mode == GAME_STATE_PLAYING) {
			// Apply input
			update_controls();

			// Update pipe
			update_pipe();

			// Update player
			update_player(0);

			// Update background
			update_background();

		} else if (game_mode == GAME_STATE_DEAD) {
			// Update player anim only
			update_player(1);

			if (key_hit(KEY_A)|key_hit(KEY_START)) {
				next_game_mode = GAME_STATE_MENU;
				audio_play(3
					, (s8*)&_binary_sfx_die_raw_start
					, (u32)&_binary_sfx_die_raw_size
					, 10512, 2, 0 );
			}
		}

		// Copy ERAM object shadows to VRAM
		oam_copy(oam_mem, objects, OAM_OBJ_SIZE);

		frame++;
	}
	return 0;
}
