#ifndef _AUDIO_H
#define _AUDIO_H

#include "memory.h"

// Additional audio spec: http://belogic.com/gba/

// Sound 1 (Square wave generator) controls
#define REG_SOUND1CNT_L *((volatile u16*)(IO_BASE+0x0060))
#define REG_SOUND1CNT_H *((volatile u16*)(IO_BASE+0x0062))
#define REG_SOUND1CNT_X *((volatile u16*)(IO_BASE+0x0064))

typedef enum SOUND1CNT_L_BITS {
	SWEEP_NUM_MASK	=	BIT_MASK(0,2),
	SWEEP_UP		=	BIT_OFF(3),
	SWEEP_DOWN		=	BIT(3),
	SWEEP_TIME_MASK	=	BIT_MASK(4,6),

	SWEEP_DISABLE	=	(0<<4),
	SWEEP_7_8_MS	=	(1<<4),
	SWEEP_15_6_MS	=	(2<<4),
	SWEEP_23_4_MS	=	(3<<4),
	SWEEP_31_3_MS	=	(4<<4),
	SWEEP_39_1_MS	=	(5<<4),
	SWEEP_46_9_MS	=	(6<<4),
	SWEEP_54_7_MS	=	(7<<4)
} SOUND1CNT_L_BITS;

typedef enum SOUND1CNT_H_BITS {
	SND_LENGTH_MASK			=	BIT_MASK(0,5),
	DUTY_PATTERN_12_5		=	(0<<6),
	DUTY_PATTERN_25			=	(1<<6),
	DUTY_PATTERN_50			=	(2<<6),
	DUTY_PATTERN_75			=	(3<<6),
	ENVELOPE_STEP_TIME_MASK	=	BIT_MASK(8,10),
	ENVELOPE_DOWN			=	BIT_OFF(11),
	ENVELOPE_UP				=	BIT(11),
	ENVELOPE_INIT_VOL_MASK	=	BIT_MASK(12, 15)
} SOUND1CNT_H_BITS;

typedef enum SOUND1CNT_X_BITS {
	SND_FREQ_MASK		=	BIT_MASK(0,10),
	SND_LOOP			=	BIT_OFF(14),
	SND_ONE_SHOT		=	BIT(14),
	SND_RESET			=	BIT(15)
} SOUND1CNT_X_BITS;

#define SOUND1CNT_L_SWEEP_NUM(n) 			(n&SWEEP_NUM_MASK)
#define SOUND1CNT_L_SWEEP_TIME(t) 			((t<<7)&SWEEP_TIME_MASK)
#define SOUND1CNT_H_SND_LEN(l)				(l&SND_LENGTH_MASK)
#define SOUND1CNT_H_ENVELOPE_STEP_TIME(t)	((t<<8)&ENVELOPE_STEP_TIME_MASK)
#define SOUND1CNT_H_ENVOLOPE_INIT_VOL(v)	((v<<12)&ENVELOPE_INIT_VOL_MASK)
#define SOUND1CNT_X_SND_FREQ(f)				(f&SND_FREQ_MASK)

// Sound 1-4 Output level and Stereo control
#define REG_SOUNDCNT_L		*((volatile u16*)(IO_BASE+0x0080))

// Direct Sound control and Sound 1-4 output ratio
#define REG_SOUNDCNT_H		*((volatile u16*)(IO_BASE+0x0082))

typedef enum SOUNDCNT_L_BITS {
	LEFT_VOL_MASK 		=	0x07,
	RIGHT_VOL_MASK		=	(0x07<<4),

	SDMG_LSQR1			=	BIT(8),
	SDMG_LSQR2			=	BIT(9),
	SDMG_LWAVE			=	BIT(10),
	SDMG_LNOISE			=	BIT(11),

	SDMG_RSQR1			=	BIT(12),
	SDMG_RSQR2			=	BIT(13),
	SDMG_RWAVE			=	BIT(14),
	SDMG_RNOISE			=	BIT(15)
} SOUNDCNT_L_BITS;

typedef enum SOUNDCNT_H_BITS {
	SDS_DMG25			=	0,
	SDS_DMG50			=	1,
	SDS_DMG100			=	2,

	SDS_A50				= 	(0<<2),
	SDS_A100			= 	(1<<2),
	SDS_B50				= 	(0<<3),
	SDS_B100			= 	(1<<3),

	SDS_AR				= 	BIT(8),
	SDS_AL				= 	BIT(9),
	SDS_ATMR0			=	(0<<10),
	SDS_ATMR1			=	(1<<10),
	SDS_ARESET			=	BIT(11),

	SDS_BR				=	BIT(12),
	SDS_BL				= 	BIT(13),
	SDS_BTMR0			= 	(0<<14),
	SDS_BTMR1			= 	(1<<14),
	SDS_BRESET			= 	BIT(15)
} SOUNDCNT_H_BITS;

// [0-7]
#define SOUNDCNT_L_SET_VOL(lvol, rvol) (lvol&LEFT_VOL_MASK)|((rvol<<4)&RIGHT_VOL_MASK)

// Master sound enable and Sound 1-4 play status
#define REG_SOUNDCNT_X *((volatile u16*)(IO_BASE+0x0084))

typedef enum SOUNDCNT_X_BITS {
	DMG_S1_ON		= BIT(0),
	DMG_S2_ON		= BIT(1),
	DMG_S3_ON		= BIT(2),
	DMG_S4_ON		= BIT(3),
	SOUND_ENABLE	= BIT(7)
} SOUNDCNT_X_BITS;

// 	Sound bias and Amplitude resolution control
#define REG_SOUNDBIAS *((volatile u16*)(IO_BASE+0x0088))

// Direct Sound channel A samples 0-3
// Lower addresses played first
#define REG_FIFO_A	((u32*)(IO_BASE+0x00A0))

// 	Direct Sound channel B samples 0-3
#define REG_FIFO_B	((u32*)(IO_BASE+0x00A4))

#define AUDIO_SAMPLE_RATE 10512
#define AUDIO_SAMPLES_PER_FRAME 176
#define	AUDIO_TM0_START 63940
#define AUDIO_MAX_CHANNELS 4

typedef struct _SOUND_VARS
{
   s8 *mixBufferBase;
   s8 *curMixBuffer;
   u32 mixBufferSize;
   u8 activeBuffer;

} SOUND_VARS;

typedef struct _SOUND_CHANNEL
{
	s8 *data;   	// pointer to the raw sound data in ROM
	u32 pos;    	// current position in the data (22.10 fixed-point)
	u32 length; 	// length of the whole sound (22.10 fixed-point)
	u32	stride;		// (22.10 fixed-point)
	u32	vol;		// (64 values, 1.6 fixed-point)
	u32	loopLength; // (22.10 fixed-point)

} SOUND_CHANNEL;

extern SOUND_CHANNEL	audio_channels[AUDIO_MAX_CHANNELS];
extern SOUND_VARS		audio_vars;

// Mixer control
void audio_init();
void audio_vsync();
void audio_mix();
void audio_play(u32 channel, s8 *data, u32 length, u32 freq, u32 vol, u32 loop);

#endif
